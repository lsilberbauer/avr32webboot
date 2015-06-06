/**************************************************************************************************
 *
 * @file ssi_handlers.h
 *
 * @brief Handlers for ssi-tags
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#ifndef SSI_HANDLERS_H_
#define SSI_HANDLERS_H_

#include <stdint.h>

/*************************************************************************************************
 * ssi_handler_application_info
 *
 * Creates: Application name and version
 *
 * parameters: see ssi_handler typedef
 */
uint16_t ssi_handler_application_info(char *buffer, uint32_t buffer_length, uint16_t current_tag_part, uint16_t *next_tag_part);

/*************************************************************************************************
 * ssi_handler_task_list
 *
 * Creates: List of FreeRTOS tasks
 *
 * parameters: see ssi_handler typedef
 */
uint16_t ssi_handler_task_list(char *buffer, uint32_t buffer_length, uint16_t current_tag_part, uint16_t *next_tag_part);

/*************************************************************************************************
 * ssi_handler_firmware_status
 *
 * Creates: Enables the upload of a new firmware and allows to confirm or reject it
 *
 * parameters: see ssi_handler typedef
 */
uint16_t ssi_handler_firmware_status(char *buffer, uint32_t buffer_length, uint16_t current_tag_part, uint16_t *next_tag_part);

/*************************************************************************************************
 * ssi_handler_registry_content
 *
 * Creates: Registry list
 *
 * parameters: see ssi_handler typedef
 */
uint16_t ssi_handler_registry_content(char *buffer, uint32_t buffer_length, uint16_t current_tag_part, uint16_t *next_tag_part);

#endif  /* SSI_HANDLERS_H_ */
