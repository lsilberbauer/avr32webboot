/**************************************************************************************************
 *
 * @file http_post_handlers.h
 *
 * @brief Handlers for http-post-actions
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#ifndef HTTP_POST_HANDLERS_H_
#define HTTP_POST_HANDLERS_H_

#include <stdint.h>

/*************************************************************************************************
 * http_post_handler_keep_new_firmware
 *
 * Keeps the newly loaded firmware
 *
 * parameters: see http_post_action_handler typedef
 */
void http_post_handler_keep_new_firmware(char *buffer, uint32_t buffer_length);

/*************************************************************************************************
 * http_post_handler_reboot
 *
 * Reboots the device
 *
 * parameters: see http_post_action_handler typedef
 */
void http_post_handler_reboot(char *buffer, uint32_t buffer_length);

/*************************************************************************************************
 * http_post_handler_registry_set
 *
 * Modifies a value in the registry
 *
 * parameters: see http_post_action_handler typedef
 */
void http_post_handler_registry_set(char *buffer, uint32_t buffer_length);

#endif /* HTTP_POST_HANDLERS_H_ */