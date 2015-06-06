/**************************************************************************************************
 *
 * @file firmware_update.c
 *
 * @brief Updates the kernel and application firmware
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#include "firmware_update.h"

#include <string.h>
#include <stdio.h>
#include "evk1100.h"
#include "wdt.h"
#include "eeprom/eeprom.h"
#include "log/log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "registry/registry.h"

static const char *module_name = "firmware_update";

static firmware_update_status current_firmware_update_status = IDLE;

static uint32_t EEPROM_current_address = 0;
static uint8_t *EEPROM_buffer = NULL;
static uint32_t EEPROM_buffer_index = 0;
static bool srec_received_correctly = false;


/*************************************************************************************************
 * firmware_parse_srec_line
 *
 * Parses one line of a SREC file, loosely based on
 * http://www.x-ways.net/winhex/kb/ff/Motorola-S2.txt and writes it in binary form to the EEPROM if it
 * has a valid checksum.
 *
 * See also http://en.wikipedia.org/wiki/SREC_%28file_format%29
 *
 * @param line
 * @return true = success, false = failure
 */
static uint8_t firmware_parse_srec_line(char *line)
{
	int byte_count = 0;
	uint32_t address = 0;
	uint8_t overhead_length = 0;  // length of overhead (addr + chksum) bytes
	uint8_t calculated_checksum = 0;         // checksum of addr, count, & data length
	int received_checksum = 0;
	int current_byte = 0;
	uint8_t conversion_results = 0;
	uint8_t i = 0;
	uint8_t empty_byte = 0xff;

	/* a line must always start with letter 'S' */
	if (*line != 'S')
	{
		log_report(UECU_LOG_ERROR, module_name, "Line in .srec file did not start with 'S'.");
		return false;
	}

    /* examine 2nd character on the line */
	switch ( *(line+1) )
	{
		case '0':
			/* a header was received */
			if (EEPROM_current_address + EEPROM_buffer_index != FIRMWARE_NEW_APPLICATION_EEPROM_START)
			{
				log_report(UECU_LOG_ERROR, module_name, "Received multiple headers in .srec file.");
				return false;
			}
			else
			{
				return true; // get next line
			}
			break;

		case '3':
			/* 32 bit address field */
			conversion_results = sscanf(line, "S3%2x%8lx", &byte_count, &address );

			if (conversion_results != 2)
			{
				log_report(UECU_LOG_ERROR, module_name, "Could not parse S3 line in .srec file.");
				return false;
			}

			overhead_length = 5; // 4 address + 1 checksum
			break;

		case '7':
			/* termination received */
			conversion_results = sscanf(line, "S7%2x%8lx%2x", &byte_count, &address, &received_checksum);

			for( i = 1; i <= byte_count; i++ )
			{
				sscanf( line + i*2, "%2x", &current_byte ); // Scan a 2 hex digit byte

				calculated_checksum += (uint8_t)current_byte;
			}

			calculated_checksum ^= 0xff; // generate one's complement

			if ((conversion_results != 3) ||
			    (calculated_checksum != received_checksum) ||
				(address != FIRMWARE_START_ADDRESS))
			{
				log_report(UECU_LOG_ERROR, module_name, "Received faulty termination in .srec file.");
				return false;
			}
			else
			{
				/* flush remainder of buffer */
				eeprom_write(EEPROM_current_address, EEPROM_buffer, EEPROM_buffer_index);
				EEPROM_current_address += EEPROM_buffer_index;
				EEPROM_buffer_index = 0;
				srec_received_correctly = true;

				return true;
			}
			break;

		default:
			log_report(UECU_LOG_ERROR, module_name, "Received unknown record in .srec file.");
			return false;
	}

	if (address < FIRMWARE_START_ADDRESS)
	{
		/* cut off trampoline code */
		return true; // get next line
	}
	else if (address - FIRMWARE_START_ADDRESS >
	         EEPROM_current_address + EEPROM_buffer_index - FIRMWARE_NEW_APPLICATION_EEPROM_START)
	{
		/* flush current state of buffer */
		eeprom_write(EEPROM_current_address, EEPROM_buffer, EEPROM_buffer_index);
		EEPROM_current_address += EEPROM_buffer_index;
		EEPROM_buffer_index = 0;

		/* fill blank sections with 0xff */
		while (address - FIRMWARE_START_ADDRESS >
		       EEPROM_current_address - FIRMWARE_NEW_APPLICATION_EEPROM_START)
		{
			eeprom_write(EEPROM_current_address, &empty_byte, 1);
			EEPROM_current_address++;
		}
	}
	else if (address - FIRMWARE_START_ADDRESS <
	         EEPROM_current_address + EEPROM_buffer_index - FIRMWARE_NEW_APPLICATION_EEPROM_START)
	{
		log_report(UECU_LOG_ERROR, module_name, "Discontinuity detected in .srec file.");
		return false;
	}


	for( i = 1; i <= byte_count; i++ )
	{
		sscanf( line + i*2, "%2x", &current_byte ); // Scan a 2 hex digit byte
		calculated_checksum += (uint8_t)current_byte;

		if (i > overhead_length) // if a data byte was scanned
		{
			if (EEPROM_buffer_index < EEPROM_PAGE_SIZE)
			{
				EEPROM_buffer[EEPROM_buffer_index] = (uint8_t)current_byte;
				EEPROM_buffer_index++;
			}
			else
			{
				/* EEPROM_buffer is full, flush it */
				eeprom_write(EEPROM_current_address, EEPROM_buffer, EEPROM_buffer_index);
				EEPROM_current_address += EEPROM_buffer_index;

				EEPROM_buffer[0] = (uint8_t)current_byte;
				EEPROM_buffer_index = 1;
			}
		}
	}

	calculated_checksum ^= 0xff; // generate one's complement

	sscanf( line + 2 + byte_count*2, "%2x", &current_byte ); // Scan the checksum
	received_checksum = (unsigned char)current_byte;

	if (calculated_checksum != received_checksum)
	{
		 log_report(UECU_LOG_ERROR, module_name, "Checksum error in .srec file.");
		 return false;
	}

	return true;
}

/*************************************************************************************************
 * firmware_backup_and_reboot_task
 *
 * Creates a backup of the current firmware in EEPROM and reboots the device afterwards
 */
static portTASK_FUNCTION( firmware_backup_and_reboot_task, pvParameters )
{
	uint32_t current_flash_address = FIRMWARE_START_ADDRESS;
	uint32_t last_used_flash_address = 0;
	uint8_t buffer[EEPROM_PAGE_SIZE];
	uint32_t buffer_index = 0;
	wdt_opt_t wdt_opt = { .us_timeout_period = 1000000  };

	/* find last used byte in flash */
	for (last_used_flash_address = FIRMWARE_END_ADDRESS;
	     last_used_flash_address > FIRMWARE_START_ADDRESS;
		 last_used_flash_address--)
	{
		if (*((uint8_t *) (last_used_flash_address)) != 0xFF)
		{
			break;
		}
	}

	registry_set_value("Firmware Backup Length", last_used_flash_address - FIRMWARE_START_ADDRESS + 1);

	while (current_flash_address + buffer_index < last_used_flash_address + 1)
	{
		if (buffer_index < EEPROM_PAGE_SIZE)
		{
			buffer[buffer_index] = *((uint8_t *) (current_flash_address + buffer_index));
			buffer_index++;
		}
		else
		{
			/* EEPROM_buffer is full, flush it */
			eeprom_write(FIRMWARE_BACKUP_EEPROM_START + (current_flash_address - FIRMWARE_START_ADDRESS), buffer, buffer_index);
			current_flash_address += buffer_index;

			buffer[0] = *((uint8_t *) current_flash_address);
			buffer_index = 1;
		}
	}

	/* flush remainder of buffer */
	eeprom_write(FIRMWARE_BACKUP_EEPROM_START + (current_flash_address - FIRMWARE_START_ADDRESS), buffer, buffer_index);

	registry_set_value("Firmware Update State", FIRMWARE_STATE_FLASH_NEW_APPLICATION); // new application ready in EEPROM, backup complete

	/* generate reset */
	Disable_global_interrupt();
	cpu_irq_disable();
	wdt_enable(&wdt_opt);
	while (1);

	/* Kill this task (redundant, I know) */
	vTaskDelete(NULL);
}

/*************************************************************************************************
 * firmware_update_init
 *
 * Initializes the firmware update module
 */
void firmware_update_init(void)
{
	registry_initialize_key("Firmware Update State", FIRMWARE_STATE_JUMP_TO_APPLICATION, true);
	registry_initialize_key("Firmware Length", 0, true);
	registry_initialize_key("Firmware Backup Length", 0, true);

	if (registry_get_value("Firmware Update State") == FIRMWARE_STATE_RESTORE_BACKUP)
	{
		/* FIRMWARE_STATE_RESTORE_BACKUP is activated automatically after a new version has been flashed */
		current_firmware_update_status = SUCCESSFUL_ASK_USER_TO_KEEP;
	}
}

/*************************************************************************************************
 * firmware_update_get_status
 *
 * Returns the current status of the firmware update
 */
firmware_update_status firmware_update_get_status(void)
{
	return current_firmware_update_status;
}

/*************************************************************************************************
 * firmware_update_set_status_idle
 *
 * Resets the firmware update state machine
 */
void firmware_update_set_status_idle(void)
{
	current_firmware_update_status = IDLE;
}

/*************************************************************************************************
 * firmware_update_application_start
 *
 * Initializes the application update routine
 */
void firmware_update_application_start(void)
{
	current_firmware_update_status = IDLE;

	EEPROM_current_address = FIRMWARE_NEW_APPLICATION_EEPROM_START;

	EEPROM_buffer = pvPortMalloc(EEPROM_PAGE_SIZE);
	EEPROM_buffer_index = 0;

	srec_received_correctly = false;
}

/*************************************************************************************************
 * firmware_update_application_do_work
 *
 * Receives .hex file data, parses it, checks it and stores it in the EEPROM buffer if everything looks good
 *
 * @param *buffer
 * @param buffer_length
 */
void firmware_update_application_do_work(char *buffer, uint32_t buffer_length)
{
	static char current_line[50];
	static uint8_t current_line_index = 0;
	uint32_t i = 0;

	if (current_firmware_update_status != UPDATE_FAILED)
	{
		for (i < 0; i < buffer_length; i++)
		{
			if (current_line_index < sizeof(current_line))
			{
				current_line[current_line_index] = *(buffer + i);
			}
			else
			{
				log_report(UECU_LOG_ERROR, module_name, "Line length in .srec file exceeds buffer.");
				current_firmware_update_status = UPDATE_FAILED;
				break;
			}

			if (current_line[current_line_index] == '\n')
			{
				if (current_line_index < 2)
				{
					/* ignore blank lines */
				}
				else if (firmware_parse_srec_line(current_line) == false)
				{
					current_firmware_update_status = UPDATE_FAILED;
					break;
				}

				current_line_index = 0;
			}
			else
			{
				current_line_index++;
			}
		}

		/* the remainder of current_line is processed next time */
	}
}

/*************************************************************************************************
 * firmware_update_application_finish
 *
 * Checks the newly received application and starts it everything is ok
 */
void firmware_update_application_finish(void)
{
	if (srec_received_correctly)
	{
		current_firmware_update_status = SUCCESSFUL_REBOOTING;
		registry_set_value("Firmware Length", EEPROM_current_address - FIRMWARE_NEW_APPLICATION_EEPROM_START);

		/* create backup and reboot */
		xTaskCreate(firmware_backup_and_reboot_task, ( const char * const) "BACKUP",
		        	configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 1 , ( xTaskHandle * )NULL );
	}
	else
	{
		current_firmware_update_status = UPDATE_FAILED;
	}

	vPortFree(EEPROM_buffer);
}
