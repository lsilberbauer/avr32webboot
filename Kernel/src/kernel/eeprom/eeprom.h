/**************************************************************************************************
 *
 * @file eeprom.h
 *
 * @brief Initializes and controls the external eeprom
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>
#include "at45dbx.h"

/* needed for compatibility with other modules */
#define EEPROM_PAGE_SIZE			AT45DBX_SECTOR_SIZE


/*************************************************************************************************
 * eeprom_init
 *
 * Initializes the external eeprom (used for compatibility with other projects)
 */
void eeprom_init(void);

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
void eeprom_write(uint32_t address, uint8_t *buffer, uint32_t length);

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
void eeprom_read(uint32_t address, uint8_t *buffer, uint32_t length);

#endif  /* EEPROM_H_ */
