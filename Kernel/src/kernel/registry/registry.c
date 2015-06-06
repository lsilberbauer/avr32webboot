/**************************************************************************************************
 *
 * @file registry.c
 *
 * @brief Provides a non-volatile registry for storing parameters and accessing them by name
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#include "registry.h"
#include "eeprom/eeprom.h"
#include "log/log.h"
#include <string.h>

/* based on http://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)HashTables.html */

static uint32_t registry_size = 0;
static const char *module_name = "registry";

/*************************************************************************************************
 * registry_calculate_hash
 *
 * Calculates a hash value to a given string
 *
 * @param key
 * @return hash value
 */
static uint32_t registry_calculate_hash(const char *key)
{
	uint32_t hash = 0;
	uint8_t i = 0;

	for (i = 0; i < REGISTRY_KEY_LENGTH; i++)
	{
		if (key[i] == 0)
		{
			/* null termination of string found */
			break;
		}

		hash = hash * REGISTRY_HASH_MULTIPLIER + key[i];
	}

	return hash;
}

/*************************************************************************************************
 * registry_find_key
 *
 * Searches the registry for a given key
 *
 * @param key
 * @param value
 * @param location (hash)
 * @return true if key was found, else false
 */
static bool registry_find_key(const char *key, uint32_t *value, uint32_t *location)
{
	uint32_t hash = 0;
	uint32_t search_start_address = 0;
	bool key_is_found = false;
	REGISTRY_ENTRY registry_entry;

	hash = registry_calculate_hash(key);
	search_start_address = hash % REGISTRY_SIZE;

	do
	{
		eeprom_read((hash % REGISTRY_SIZE) * sizeof(REGISTRY_ENTRY), (uint8_t *)&registry_entry, sizeof(registry_entry));

		if (registry_entry.key[0] == 0)
		{
			return false; // location is empty
		}
		else if (strncmp(key, registry_entry.key, REGISTRY_KEY_LENGTH) == 0)
		{
			key_is_found = true;
			*value = registry_entry.value;
			*location = hash % REGISTRY_SIZE;
			return true;
		}
		else
		{
			hash++;
		}
	} while ((!key_is_found) && (hash % REGISTRY_SIZE != search_start_address));

	return false;
}

/*************************************************************************************************
 * registry_create_key
 *
 * Creates a new key in the registry
 *
 * @param key
 * @param value
 * @param readonly
 */
static void registry_create_key(const char *key, uint32_t new_value, bool readonly)
{
	uint32_t hash = 0;
	bool free_space_found = false;
	REGISTRY_ENTRY registry_entry;

	if (registry_size < REGISTRY_SIZE)
	{
		hash = registry_calculate_hash(key);

		while (!free_space_found)
		{
			eeprom_read((hash % REGISTRY_SIZE) * sizeof(REGISTRY_ENTRY), (uint8_t *)&registry_entry, sizeof(registry_entry));

			if (registry_entry.key[0] == 0)
			{
				free_space_found = true;

				strncpy(registry_entry.key, key, REGISTRY_KEY_LENGTH);
				registry_entry.value = new_value;
				registry_entry.readonly = readonly;

				eeprom_write((hash % REGISTRY_SIZE) * sizeof(REGISTRY_ENTRY), (uint8_t *)&registry_entry, sizeof(registry_entry));

				registry_size++;

				/* skip the string and read-only field and write the value directly */
				eeprom_write(REGISTRY_START_ADDRESS + REGISTRY_KEY_LENGTH + 1, (uint8_t *)&registry_size, 4);
			}
			else
			{
				hash++;
			}
		}
	}
	else
	{
		log_report(UECU_LOG_FATAL, module_name, "Registry is full!");
	}

}

/*************************************************************************************************
 * registry_init
 *
 * Initializes the registry
 *
 * @pre EEPROM must be initialized
 *
 * @reset resets the entire registry to its default state
 */
void registry_init(bool reset)
{
	uint32_t i = 0;
	REGISTRY_ENTRY registry_entry;

	if (reset)
	{
		for (i = 0; i < REGISTRY_KEY_LENGTH; i++)
		{
			registry_entry.key[i] = 0;
		}

		registry_entry.readonly = 0;
		registry_entry.value = 0;

		for (i = 0; i < REGISTRY_SIZE; i++)
		{
			eeprom_write(i * sizeof(REGISTRY_ENTRY), (uint8_t *)&registry_entry, sizeof(registry_entry));
		}

		registry_size = 0;
	}
	else
	{
		/* find out how many keys are in use */

		for (i = 0; i < REGISTRY_SIZE; i++)
		{
			eeprom_read(i * sizeof(REGISTRY_ENTRY), (uint8_t *)&registry_entry, sizeof(registry_entry));

			if (registry_entry.key[0] != 0)
			{
				registry_size++;
			}
		}
	}
}

/*************************************************************************************************
 * registry_initialize_key
 *
 * Checks if the key is already present, and if not, sets its default value.
 *
 * Should be called by the init function of the respective module for all used keys.
 *
 * @key to look for
 * @default_value
 * @readonly true if value is read-only
 */
void registry_initialize_key(char *key, uint32_t default_value, bool readonly)
{
	uint32_t value = 0;
	uint32_t location = 0;

	if (strlen(key) > REGISTRY_KEY_LENGTH)
	{
		log_report(UECU_LOG_WARN, module_name, "Key '%s' too long, truncated.", key);
	}

	if (registry_find_key(key, &value, &location) == false)
	{
		registry_create_key(key, default_value, readonly);
	}
}

/*************************************************************************************************
 * registry_get_value
 *
 * Retrieves a value from the registry
 *
 * @key to look for
 *
 * @returns the value of the key. Throws a fatal error if something goes wrong.
 */
uint32_t registry_get_value(char *key)
{
	uint32_t return_value = 0;
	uint32_t location = 0;

	if (registry_find_key(key, &return_value, &location) == false)
	{
		log_report(UECU_LOG_FATAL, module_name, "Key '%s' not found!", key);
	}

	return return_value;
}

/*************************************************************************************************
 * registry_set_value
 *
 * Sets a value in the registry
 *
 * @key to look for
 * @new_value
 */
void registry_set_value(char *key, uint32_t new_value)
{
	uint32_t value = 0;
	uint32_t location = 0;

	if (registry_find_key(key, &value, &location) == false)
	{
		log_report(UECU_LOG_FATAL, module_name, "Key '%s' not found!", key);
	}
	else
	{
		/* skip the string and read-only field and write the value directly */
		eeprom_write(location * sizeof(REGISTRY_ENTRY) + REGISTRY_KEY_LENGTH + 1, (uint8_t *)&new_value, 4);
	}
}

/*************************************************************************************************
 * registry_get_size
 *
 * Returns the number of keys currently in use
 *
 * @return
 */
uint32_t registry_get_size(void)
{
	return registry_size;
}

/*************************************************************************************************
 * registry_dump
 *
 * Dumps the registry into an array
 *
 * @registry_array pointer to array which is filled
 * @registry_array_size numbers of elements the array can hold
 */
void registry_dump(REGISTRY_ENTRY * const registry_array, uint32_t registry_array_size)
{
	uint32_t i = 0;
	REGISTRY_ENTRY registry_entry;
	uint32_t array_index = 0;

	for (i = 0; i < REGISTRY_SIZE; i++)
	{
		eeprom_read(i * sizeof(REGISTRY_ENTRY), (uint8_t *)&registry_entry, sizeof(registry_entry));

		if ((registry_entry.key[0] != 0) && (array_index < registry_array_size))
		{
			strncpy(registry_array[array_index].key, registry_entry.key, REGISTRY_KEY_LENGTH);
			registry_array[array_index].readonly = registry_entry.readonly;
			registry_array[array_index].value = registry_entry.value;
			array_index++;
		}
	}
}

