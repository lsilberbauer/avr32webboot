/**************************************************************************************************
 *
 * @file post.h
 *
 * @brief Handles HTTP-Post requests, used for firmware updates
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#include <string.h>
#include "http_post/http_post.h"
#include "firmware_update/firmware_update.h"
#include "log/log.h"

static const char *module_name = "http_post";

static http_post_type current_post_type = UNKNOWN;

static char **action_names;
static http_post_action_handler *handlers;
static uint32_t number_of_action_names = 0;


/*************************************************************************************************
 * httpd_post_begin
 *
 * Called when a POST request has been received. The application can decide
 * whether to accept it or not.
 *
 * @param connection Unique connection identifier, valid until httpd_post_end
 *        is called.
 * @param uri The HTTP header URI receiving the POST request.
 * @param http_request The raw HTTP request (the first packet, normally).
 * @param http_request_len Size of 'http_request'.
 * @param content_len Content-Length from HTTP header.
 * @param response_uri Filename of response file, to be filled when denying the
 *        request
 * @param response_uri_len Size of the 'response_uri' buffer.
 * @param post_auto_wnd Set this to 0 to let the callback code handle window
 *        updates by calling 'httpd_post_data_recved' (to throttle rx speed)
 *        default is 1 (httpd handles window updates automatically)
 * @return ERR_OK: Accept the POST request, data may be passed in
 *         another err_t: Deny the POST request, send back 'bad request'.
 */
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd)
{
	current_post_type = UNKNOWN;

	return ERR_OK;
}

/*************************************************************************************************
 * httpd_post_receive_data
 *
 * Called for each pbuf of data that has been received for a POST.
 * ATTENTION: The application is responsible for freeing the pbufs passed in!
 *
 * @param connection Unique connection identifier.
 * @param p Received data.
 * @return ERR_OK: Data accepted.
 *         another err_t: Data denied, http_post_get_response_uri will be called.
 */
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
	#define ACTION_IDENTIFIER		   "action="
	#define FIRMWARE_IDENTIFIER        "firmware"
	#define NAME_DELIMITER_START       "name=\""
	#define CONTENT_DELIMITER_START    "\r\n\r\n"
	#define CONTENT_DELIMITER_END      "\r\n----"

	char *form_field_name = NULL;
	char *content = NULL;
	uint32_t content_length = 0;
	struct pbuf *temp_buffer = p;
	char *end_position = NULL;
	uint32_t i = 0;

	/* iterate through the buffer struct */
	while (temp_buffer != NULL)
	{
		if (current_post_type == UNKNOWN)
		{
			if (!strncmp(temp_buffer->payload, ACTION_IDENTIFIER, sizeof(ACTION_IDENTIFIER) - 1))
			{
				/* if the keyword "action" is present, we assume that the post request is application/x-www-form-urlencoded */

				content = (char *)temp_buffer->payload + sizeof(ACTION_IDENTIFIER) - 1;

				for (i = 0; i < number_of_action_names; i++)
				{
					if (!strncmp(content, action_names[i], strlen(action_names[i])))
					{
						/* call the action handler */
						handlers[i](content, temp_buffer->len - (sizeof(ACTION_IDENTIFIER) - 1));
						current_post_type = ACTION;
						break;
					}
				}
			}
			else
			{
				/* this must be the start of a multipart/form-data request, we need to strip off the header */
				content = strstr(temp_buffer->payload, CONTENT_DELIMITER_START) + sizeof(CONTENT_DELIMITER_START) - 1;
				content_length = temp_buffer->len - (content - (char *)temp_buffer->payload);

				form_field_name = strstr(temp_buffer->payload, NAME_DELIMITER_START) + sizeof(NAME_DELIMITER_START) - 1;

				/* check if a firmware update form was submitted */
				if (!strncmp(form_field_name, FIRMWARE_IDENTIFIER, sizeof(FIRMWARE_IDENTIFIER) - 1))
				{
					current_post_type = FIRMWARE_UPDATE;
					firmware_update_application_start();
				}
			}
		}
		else
		{
			content = temp_buffer->payload;
			content_length = temp_buffer->len;
		}

		/* check if end is reached */
		end_position = strnstr(content, CONTENT_DELIMITER_END, content_length);

		if (end_position > 0)
		{
			content_length = end_position - content;
		}

		switch (current_post_type)
		{
			case FIRMWARE_UPDATE:
				firmware_update_application_do_work(content, content_length);
				break;

			case ACTION:
				// do nothing, handler has already been called
			break;

			default:
				return ERR_VAL;
				break;
		}

		temp_buffer = temp_buffer->next;
	}

	pbuf_free(p);

	return ERR_OK;
}

/*************************************************************************************************
 * httpd_post_finished
 *
 * Called when all data is received or when the connection is closed.
 * The application must return the filename/URI of a file to send in response
 * to this POST request. If the response_uri buffer is untouched, a 404
 * response is returned.
 *
 * @param connection Unique connection identifier.
 * @param response_uri Filename of response file, to be filled when denying the request
 * @param response_uri_len Size of the 'response_uri' buffer.
 *
 */
void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
{
	const char my_response_uri[] = "/index.shtml";

	if (response_uri_len >= (sizeof(my_response_uri) - 1))
	{
		strncpy(response_uri, my_response_uri, (sizeof(my_response_uri) - 1));
	}
	else
	{
		configASSERT(0);
	}

	switch (current_post_type)
	{
		case FIRMWARE_UPDATE:
				firmware_update_application_finish();
			break;

		default:
			/* do nothing */
			break;
	}
}

/*************************************************************************************************
 * http_post_register_handler
 *
 * Registers a function to be called when the keyword "action" was submitted to the server. The word
 * after the keyword "action" defines which action should be taken (example: action=dothis&... where
 * dothis is the action_name in this case.
 *
 * @param action_name, max length: 20 characters including "\0" termination, case sensitive!
 * @param function_pointer
 */
void http_post_register_handler(const char *action_name, http_post_action_handler function_pointer)
{
	if (strlen(action_name) > HTTP_POST_ACTION_NAME_MAX_LENGTH)
	{
		log_report(UECU_LOG_ERROR, module_name, "action-name %s too long, truncated", action_name);
	}

	/* Make tag array bigger to fit the new keyword
	 * Note: see here http://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc
	 */
	action_names = pvPortRealloc(action_names, (number_of_action_names + 1) * sizeof(char *));

	if (action_names != NULL)
	{
		action_names[number_of_action_names] = action_name;
	}

	handlers = pvPortRealloc(handlers, (number_of_action_names + 1) * sizeof(handlers));

	if (handlers != NULL)
	{
		handlers[number_of_action_names] = function_pointer;
	}

	number_of_action_names++;
}