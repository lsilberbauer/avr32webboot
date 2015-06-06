/**************************************************************************************************
 *
 * @file eeprom.c
 *
 * @brief Initializes and controls the external eeprom
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#include "eeprom/eeprom.h"
#include "gpio.h"
#include "evk1100.h"
#include "common_nvm.h"
#include "log/log.h"

#ifdef FREERTOS_USED
	#include "FreeRTOS.h"
#endif

static const char *module_name = "eeprom";

/*************************************************************************************************
 * eeprom_init
 *
 * Initializes the external eeprom (used for compatibility with other projects)
 */
void eeprom_init(void)
{
	static const gpio_map_t AT45DBX_SPI_GPIO_MAP =
	{
		{AT45DBX_SPI_SCK_PIN,          AT45DBX_SPI_SCK_FUNCTION         },  // SPI Clock.
		{AT45DBX_SPI_MISO_PIN,         AT45DBX_SPI_MISO_FUNCTION        },  // MISO.
		{AT45DBX_SPI_MOSI_PIN,         AT45DBX_SPI_MOSI_FUNCTION        },  // MOSI
		{AT45DBX_SPI_NPCS0_PIN,		   AT45DBX_SPI_NPCS0_FUNCTION       },  // CS
	};

	gpio_enable_module(AT45DBX_SPI_GPIO_MAP, sizeof(AT45DBX_SPI_GPIO_MAP) / sizeof(AT45DBX_SPI_GPIO_MAP[0]));

	/* Initialize dataflash */
	at45dbx_init();
	
	/* Perform memory check */
	if (!at45dbx_mem_check())
	{
		log_report(UECU_LOG_ERROR, module_name, "Error initializing EEPROM");
	}
}

/*************************************************************************************************
 * eeprom_write
 *
 * Writes from a byte buffer to the eeprom and handles sector alignment automatically
 * (used for compatibility with other projects)
 *
 * @param address address to write to in eeprom
 * @param *buffer byte buffer
 * @param length
 */
void eeprom_write(uint32_t address, uint8_t *buffer, uint32_t length)
{
	uint32_t i = 0;
	
	#ifdef FREERTOS_USED
		/* @TODO: use semaphore here */
		portENTER_CRITICAL();
	#endif
	
	at45dbx_write_byte_open(address);

	for (i = 0; i < length; i++)
	{
		at45dbx_write_byte(buffer[i]);
	}
	
	at45dbx_write_close();
	
	#ifdef FREERTOS_USED
		portEXIT_CRITICAL();
	#endif
}

/*************************************************************************************************
 * eeprom_read
 *
 * Reads from the eeprom to a byte buffer and handles sector alignment automatically
 * (used for compatibility with other projects)
 *
 * @param address address to write to in eeprom
 * @param *buffer byte buffer, must have minimum length bytes
 * @param length
 */
void eeprom_read(uint32_t address, uint8_t *buffer, uint32_t length)
{
	uint32_t i = 0;
	
	#ifdef FREERTOS_USED
		/* @TODO: use semaphore here */
		portENTER_CRITICAL();
	#endif	
	
	at45dbx_read_byte_open(address);

	for (i = 0; i < length; i++)
	{
		buffer[i] = at45dbx_read_byte();
	}
	
	at45dbx_write_close();
	
	#ifdef FREERTOS_USED
		portEXIT_CRITICAL();
	#endif
}