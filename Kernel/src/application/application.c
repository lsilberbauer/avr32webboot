/**************************************************************************************************
 *
 * @file application.c
 *
 * @brief Contains the entry point to the application specific (non-kernel) code
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#include "application.h"

#include <stdio.h>
#include <stdint.h>
#include "registry/registry.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"

static const char *module_name = "application";

/*************************************************************************************************
 * application_task_1
 *
 * Toggles the LEDs with a user defined frequency
 */
static portTASK_FUNCTION( application_task_1, pvParameters )
{
	registry_initialize_key("Task 1 Frequency", 10, false);
	
	for(;;)
	{
		LED_Toggle(LED0);
		LED_Toggle(LED1);
		LED_Toggle(LED2);
		LED_Toggle(LED3);
		LED_Toggle(LED4);
		LED_Toggle(LED5);

		vTaskDelay((1000 / registry_get_value("Task 1 Frequency")) / portTICK_PERIOD_MS);
	}
}

/*************************************************************************************************
 * application_init
 *
 * Initializes the application specific code section of this firmware
 */
void application_init(void)
{
	TaskHandle_t application_task_handle_1 = NULL;

	xTaskCreate(application_task_1, ( const char * const) "Task 1", 1568, NULL, 1, &application_task_handle_1);
	configASSERT(application_task_handle_1);
}