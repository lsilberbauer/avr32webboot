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
#include <stdio.h>
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "kernel.h"
#include "application.h"

/*************************************************************************************************
 * main
 *
 * Main entry point. Note that some startup code is called first.
 */
int main (void)
{
	kernel_init();
	application_init();

	/* Start FreeRTOS; no interrupts must fire before this, otherwise an exception is raised */
	portDBG_TRACE("Starting FreeRTOS V8.2.0 ...");
	vTaskStartScheduler();

	/* Will only reach here if there was insufficient memory to create the idle task. */

	return 0;
}

/*************************************************************************************************
 * vApplicationStackOverflowHook
 *
 * If a stack overflow occurs, this function is called often, but not always. If it is called the
 * stack has already been corrupted and is not to be trusted anymore, so no functions shall be called.
 */
void vApplicationStackOverflowHook( xTaskHandle xTask, signed char *pcTaskName ); // prototype to prevent warning
void vApplicationStackOverflowHook( xTaskHandle xTask, signed char *pcTaskName )
{
	/* insert a breakpoint here to detect stack overflows */

	asm ("nop");

	while (true)
	{
		// loop forever
	}
}