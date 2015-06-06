/**************************************************************************************************
 *
 * @file kernel.c
 *
 * @brief Initializes the Kernel, a collection of modules to provide the webserver, firmware
 *        upload and registry functionality
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#include "kernel.h"

#include <asf.h>
#include "log/log.h"
#include "ethernet/ethernet.h"
#include "eeprom/eeprom.h"
#include "registry/registry.h"
#include "firmware_update/firmware_update.h"
#include "ssi/ssi.h"
#include "ssi_handlers/ssi_handlers.h"
#include "http_post/http_post.h"
#include "http_post_handlers/http_post_handlers.h"

/*************************************************************************************************
 * kernel_init
 *
 * Initializes the modules required by the kernel
 */
void kernel_init(void)
{
	volatile bool reset_registry = false; // set it to true via the debugger for wiping the registry

	/* Initialization functions */
	sysclk_init();
	log_init();
	eeprom_init();
	registry_init(reset_registry);
	firmware_update_init();
	ethernet_init();

	/* note: ssi_init() is only called after the task scheduler has been started,
	 * so it is safe to set the handlers here
	 */

	ssi_register_handler("application_info", &ssi_handler_application_info);
	ssi_register_handler("task_list", &ssi_handler_task_list);
	ssi_register_handler("firmware_status", &ssi_handler_firmware_status);
	ssi_register_handler("registry_content", &ssi_handler_registry_content);

	http_post_register_handler("firmware_keep", &http_post_handler_keep_new_firmware);
	http_post_register_handler("firmware_reject", &http_post_handler_reboot);
	http_post_register_handler("registry_set", &http_post_handler_registry_set);
	http_post_register_handler("reboot", &http_post_handler_reboot);

	registry_initialize_key("Test Key", 4, false);
}



