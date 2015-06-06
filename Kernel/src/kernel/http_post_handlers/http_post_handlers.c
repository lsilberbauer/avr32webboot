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

#include <stdint.h>
#include <string.h>
#include "webserver/httpd.h"
#include "http_post_handlers.h"
#include "wdt.h"
#include "firmware_update/firmware_update.h"
#include "registry/registry.h"
#include "log/log.h"

static const char *module_name = "http_post_handlers";

/*************************************************************************************************
 * http_post_handler_keep_new_firmware
 *
 * Keeps the newly loaded firmware
 *
 * parameters: see http_post_action_handler typedef
 */
void http_post_handler_keep_new_firmware(char *buffer, uint32_t buffer_length)
{
	registry_set_value("Firmware Update State", FIRMWARE_STATE_JUMP_TO_APPLICATION);

	firmware_update_set_status_idle();
}

/*************************************************************************************************
 * http_post_handler_reboot
 *
 * Reboots the device
 *
 * parameters: see http_post_action_handler typedef
 */
void http_post_handler_reboot(char *buffer, uint32_t buffer_length)
{
	wdt_opt_t wdt_opt = { .us_timeout_period = 1000000  };

	/* generate reset, the backup will be automatically loaded */
	Disable_global_interrupt();
	cpu_irq_disable();
	wdt_enable(&wdt_opt);
	while (1);
}

/*************************************************************************************************
 * http_post_handler_registry_set
 *
 * Modifies a value in the registry
 *
 * parameters: see http_post_action_handler typedef
 */
void http_post_handler_registry_set(char *buffer, uint32_t buffer_length)
{
	char *buffer_copy = NULL;
	uint32_t conversion_results = 0;
	char *key = NULL;
	int new_value = 0;
	uint32_t i = 0;

	buffer_copy = pvPortMalloc(buffer_length + 1);
	key = pvPortMalloc(buffer_length + 1);

	buffer_copy = strncpy(buffer_copy, buffer, buffer_length);
	buffer_copy[buffer_length] = 0; // terminate string

	for (i = 0; i < buffer_length; i++)
	{
		if (buffer_copy[i] == '&')
		{
			buffer_copy[i] = ' '; // replace & by whitespaces, so that sscanf stops at the right position
		}
	}

	conversion_results = sscanf(buffer_copy, "registry_set key=%s value=%d", key, &new_value );

	if (conversion_results == 2)
	{
		for (i = 0; i < strlen(key); i++)
		{
			if (key[i] == '+')
			{
				key[i] = ' '; // replace '+' by whitespaces, to correct HTTP-POST encoding
			}
		}

		registry_set_value(key, (uint32_t)new_value);
	}
	else
	{
		log_report(UECU_LOG_ERROR, module_name, "Could not set registry key %s", key);
	}


	vPortFree(buffer_copy);
	vPortFree(key);
}