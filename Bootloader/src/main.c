/**************************************************************************************************
 *
 * @file main.c
 *
 * @brief Contains the program's main entry point
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/
#include <asf.h>
#include "gpio.h"
#include "eeprom/eeprom.h"
#include "registry/registry.h"
#include "firmware_update/firmware_update.h"

typedef void (*application_pointer)(void) __attribute__ ((noreturn));

int main (void)
{
	uint32_t firmware_update_state = 0;
	uint32_t firmware_length = 0;
	uint32_t firmware_backup_length = 0;
	uint32_t flash_address = FIRMWARE_START_ADDRESS;
	uint32_t bytes_to_read = 0;
	uint8_t *buffer = 0;
	application_pointer application_start = (application_pointer)FIRMWARE_START_ADDRESS;	

	/* Initialization functions */
	wdt_disable();
	sysclk_init();

	eeprom_init();
	registry_init(!gpio_get_pin_value(GPIO_PUSH_BUTTON_0)); // hold PB0 at startup to reset EEPROM
	nvm_init(INT_FLASH);

	registry_initialize_key("Firmware Update State", 0, true);
	registry_initialize_key("Firmware Length", 0, true);
	registry_initialize_key("Firmware Backup Length", 0, true);

	firmware_update_state = registry_get_value("Firmware Update State");
	firmware_length = registry_get_value("Firmware Length");
	firmware_backup_length = registry_get_value("Firmware Backup Length");

	switch (firmware_update_state)
	{
		case FIRMWARE_STATE_JUMP_TO_APPLICATION:
			application_start();
			break;

		case FIRMWARE_STATE_FLASH_NEW_APPLICATION:

			buffer = malloc(AVR32_FLASHC_PAGE_SIZE);

			while (flash_address - FIRMWARE_START_ADDRESS < firmware_length)
			{
				bytes_to_read = Min((firmware_length - (flash_address - FIRMWARE_START_ADDRESS)) + 1, AVR32_FLASHC_PAGE_SIZE);

				eeprom_read(FIRMWARE_NEW_APPLICATION_EEPROM_START + (flash_address - FIRMWARE_START_ADDRESS), buffer, bytes_to_read);
				nvm_write(INT_FLASH, flash_address, (void *)buffer, bytes_to_read);

				flash_address += bytes_to_read;
			}

			registry_set_value("Firmware Update State", FIRMWARE_STATE_RESTORE_BACKUP);

			free(buffer);

			application_start();
			break;

		case FIRMWARE_STATE_RESTORE_BACKUP:

			buffer = malloc(AVR32_FLASHC_PAGE_SIZE);

			while (flash_address - FIRMWARE_START_ADDRESS < firmware_backup_length)
			{
				bytes_to_read = Min((firmware_backup_length - (flash_address - FIRMWARE_START_ADDRESS)) + 1, AVR32_FLASHC_PAGE_SIZE);

				eeprom_read(FIRMWARE_BACKUP_EEPROM_START + (flash_address - FIRMWARE_START_ADDRESS), buffer, bytes_to_read);
				nvm_write(INT_FLASH, flash_address, (void *)buffer, bytes_to_read);

				flash_address += bytes_to_read;
			}

			registry_set_value("Firmware Update State", FIRMWARE_STATE_JUMP_TO_APPLICATION);

			free(buffer);

			application_start();
			break;
	}

}
