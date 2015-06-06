/**************************************************************************************************
 *
 * @file ssi_handlers.c
 *
 * @brief Handlers for ssi-tags
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#include "ssi_handlers.h"

#include <string.h>
#include <stdio.h>
#include <compiler.h>
#include "FreeRTOS.h"
#include "task.h"
#include "firmware_update/firmware_update.h"
#include "registry/registry.h"
#include "application.h"

/*************************************************************************************************
 * ssi_handler_application_info
 *
 * Creates: Application name and version
 *
 * parameters: see ssi_handler typedef
 */
uint16_t ssi_handler_application_info(char *buffer, uint32_t buffer_length, uint16_t current_tag_part, uint16_t *next_tag_part)
{
	#ifndef APPLICATION_NAME
		#define APPLICATION_NAME "Kernel"
	#endif
	#ifndef APPLICATION_VERSION
		#define APPLICATION_VERSION "V1.1"
	#endif

	#define APPLICATION_INFO APPLICATION_NAME " " APPLICATION_VERSION

	strncpy(buffer, APPLICATION_INFO, buffer_length);

	*next_tag_part = 0xFFFF;

	return (Min(strlen(APPLICATION_INFO), buffer_length));
}

/*************************************************************************************************
 * ssi_handler_task_list
 *
 * Creates: List of FreeRTOS tasks
 *
 * parameters: see ssi_handler typedef
 */
uint16_t ssi_handler_task_list(char *buffer, uint32_t buffer_length, uint16_t current_tag_part, uint16_t *next_tag_part)
{
	static uint32_t number_array_elements = 0;
	static TaskStatus_t *task_status_array = 0;
	char task_status;
	uint32_t total_length = 0;
	uint32_t length = 0;

	if (current_tag_part == 0)
	{
		number_array_elements = uxTaskGetNumberOfTasks();
		task_status_array = pvPortMalloc( number_array_elements * sizeof( TaskStatus_t ) );

		if( task_status_array != NULL )
		{
			/* Generate the (binary) data. */
			number_array_elements = uxTaskGetSystemState( task_status_array, number_array_elements, NULL );
		}
	}

	total_length = snprintf(buffer, buffer_length, "<tr><td>");
	buffer += total_length;

	/* write contents of array to output buffer */
	length = strlen(task_status_array[ current_tag_part ].pcTaskName);

	if (total_length + length <= buffer_length)
	{
		strncpy(buffer, task_status_array[ current_tag_part ].pcTaskName, length);
		total_length += length;
		buffer += length;
	}
	else
	{
		return -1;
	}

	/* the rest will never be longer then 80 characters */
	if (total_length + 80 <= buffer_length)
	{
		switch( task_status_array[ current_tag_part ].eCurrentState )
		{
			case eReady:		task_status = 'R';
								break;

			case eBlocked:		task_status = 'B';
								break;

			case eSuspended:	task_status = 'S';
								break;

			case eDeleted:		task_status = 'D';
								break;

			default:			/* Should not get here, but it is included
								to prevent static checking errors. */
								task_status = 0x00;
								break;
		}

		/* Write the rest of the string. */
		length = snprintf(buffer, 80, "</td><td>%c</td><td>%u</td><td>%u</td><td>%u</td></tr>",
								task_status,
								( unsigned int ) task_status_array[ current_tag_part ].uxCurrentPriority,
								( unsigned int ) task_status_array[ current_tag_part ].usStackHighWaterMark,
								( unsigned int ) task_status_array[ current_tag_part ].xTaskNumber );

		total_length += length;
		buffer += length;
	}
	else
	{
		return -1;
	}

	if (current_tag_part + 1 < number_array_elements)
	{
		*next_tag_part = current_tag_part + 1;
	}
	else
	{
		*next_tag_part = 0xFFFF;

		/* Free the array again. */
		vPortFree( task_status_array );
	}

	return total_length;
}

/*************************************************************************************************
 * ssi_handler_firmware_status
 *
 * Creates: Enables the upload of a new firmware and allows to confirm or reject it
 *
 * parameters: see ssi_handler typedef
 */
uint16_t ssi_handler_firmware_status(char *buffer, uint32_t buffer_length, uint16_t current_tag_part, uint16_t *next_tag_part)
{
	uint32_t total_length = 0;

	if (buffer_length > 65)
	{
		switch (firmware_update_get_status())
		{
			case IDLE:
			total_length = snprintf(buffer, 65, "<script language=\"JavaScript\">firmware_status(0);</script>");
			break;
			case SUCCESSFUL_REBOOTING:
			total_length = snprintf(buffer, 65, "<script language=\"JavaScript\">firmware_status(1);</script>");
			break;
			case SUCCESSFUL_ASK_USER_TO_KEEP:
			total_length = snprintf(buffer, 65, "<script language=\"JavaScript\">firmware_status(2);</script>");
			break;
			case UPDATE_FAILED:
			total_length = snprintf(buffer, 65, "<script language=\"JavaScript\">firmware_status(3);</script>");
			break;
			default:
			return -1;
			break;
		}
	}
	else
	{
		return -1;
	}

	return total_length;
}

/*************************************************************************************************
 * ssi_handler_registry_content
 *
 * Creates: Registry list
 *
 * parameters: see ssi_handler typedef
 */
uint16_t ssi_handler_registry_content(char *buffer, uint32_t buffer_length, uint16_t current_tag_part, uint16_t *next_tag_part)
{
	static const char *disabled_text = "disabled";
	static uint32_t number_array_elements = 0;
	static REGISTRY_ENTRY *registry_array = 0;
	uint32_t total_length = 0;
	char *enable_or_disable = NULL;

	if (current_tag_part == 0)
	{
		number_array_elements = registry_get_size();
		registry_array = pvPortMalloc( number_array_elements * sizeof( REGISTRY_ENTRY ) );

		if( registry_array != NULL )
		{
			registry_dump(registry_array, number_array_elements);
		}
	}

	if (registry_array[ current_tag_part ].readonly)
	{
		enable_or_disable = disabled_text;
	}

	/* this simple HTML table will be converted to a form by JavaScript */
	total_length = snprintf(buffer, buffer_length, "<tr><td>%s</td><td>%d</td><td>%d</td></tr>",
	                        registry_array[current_tag_part].key,
							(int) registry_array[current_tag_part].value,
							(int) registry_array[current_tag_part].readonly);
	buffer += total_length;

	if (current_tag_part + 1 < number_array_elements)
	{
		*next_tag_part = current_tag_part + 1;
	}
	else
	{
		*next_tag_part = 0xFFFF;

		/* Free the array again. */
		vPortFree( registry_array );
	}

	return total_length;
}