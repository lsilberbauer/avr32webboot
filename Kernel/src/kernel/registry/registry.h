/**************************************************************************************************
 *
 * @file registry.h
 *
 * @brief Provides a non-volatile registry for storing parameters and accessing them by name
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#ifndef REGISTRY_H_
#define REGISTRY_H_

#include <stdint.h>
#include "compiler.h"

#define REGISTRY_START_ADDRESS   0
#define REGISTRY_SIZE			 1024
#define REGISTRY_KEY_LENGTH		 27
#define REGISTRY_HASH_MULTIPLIER 97

typedef struct _EEPROM_REGISTRY_ENTRY
{
	char key[REGISTRY_KEY_LENGTH];
	bool readonly;
	uint32_t value;
} REGISTRY_ENTRY;

/*************************************************************************************************
 * registry_init
 *
 * Initializes the registry
 *
 * @pre EEPROM must be initialized
 *
 * @reset resets the entire registry to its default state
 */
void registry_init(bool reset);

/*************************************************************************************************
 * registry_initialize_key
 *
 * Checks if the key is already present, and if not, sets its default value.
 *
 * Should be called by the init function of the respective module for all used keys.
 *
 * @key to look for
 * @default_value
 * @readonly true if value cannot be changed by the user via the web-interface (only by the program)
 */
void registry_initialize_key(char *key, uint32_t default_value, bool readonly);

/*************************************************************************************************
 * registry_get_value
 *
 * Retrieves a value from the registry
 *
 * @key to look for
 *
 * @returns the value of the key. Throws a fatal error if something goes wrong.
 */
uint32_t registry_get_value(char *key);

/*************************************************************************************************
 * registry_set_value
 *
 * Sets a value in the registry
 *
 * @key to look for
 * @new_value
 */
void registry_set_value(char *key, uint32_t new_value);

/*************************************************************************************************
 * registry_get_size
 *
 * Returns the number of keys currently in use
 *
 * @return
 */
uint32_t registry_get_size(void);

/*************************************************************************************************
 * registry_dump
 *
 * Dumps the registry into an array
 *
 * @registry_array pointer to array which is filled
 * @registry_array_size numbers of elements the array can hold
 */
void registry_dump(REGISTRY_ENTRY * const registry_array, uint32_t registry_array_size);

#endif  /* REGISTRY_H_ */
