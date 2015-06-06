/**************************************************************************************************
 *
 * @file firmware_update.h
 *
 * @brief Updates the kernel and application firmware
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#ifndef FIRMWARE_UPDATE_H_
#define FIRMWARE_UPDATE_H_

#include <stdint.h>

#define FIRMWARE_START_ADDRESS					0x80004000
#define FIRMWARE_END_ADDRESS					0x8003FFFF

#define FIRMWARE_NEW_APPLICATION_EEPROM_START   0x700000
#define FIRMWARE_BACKUP_EEPROM_START			0x780000

#define FIRMWARE_STATE_JUMP_TO_APPLICATION		0
#define FIRMWARE_STATE_FLASH_NEW_APPLICATION	1
#define FIRMWARE_STATE_RESTORE_BACKUP			2

typedef enum
{
	IDLE = 0,
	SUCCESSFUL_REBOOTING = 1,
	SUCCESSFUL_ASK_USER_TO_KEEP = 2,
	UPDATE_FAILED = 3
} firmware_update_status;

/*************************************************************************************************
 * firmware_update_init
 *
 * Initializes the firmware update module
 */
void firmware_update_init(void);

/*************************************************************************************************
 * firmware_update_get_status
 *
 * Returns the current status of the firmware update
 */
firmware_update_status firmware_update_get_status(void);

/*************************************************************************************************
 * firmware_update_set_status_idle
 *
 * Resets the firmware update state machine
 */
void firmware_update_set_status_idle(void);

/*************************************************************************************************
 * firmware_update_application_start
 *
 * Initializes the application update routine
 */
void firmware_update_application_start(void);

/*************************************************************************************************
 * firmware_update_application_do_work
 *
 * Receives .hex file data, parses it, checks it and stores it in the EEPROM buffer if everything looks good
 *
 * @param *buffer
 * @param buffer_length
 */
void firmware_update_application_do_work(char *buffer, uint32_t buffer_length);

/*************************************************************************************************
 * firmware_update_application_finish
 *
 * Checks the newly received application and starts it everything is ok
 */
void firmware_update_application_finish(void);

#endif /* FIRMWARE_UPDATE_H_ */