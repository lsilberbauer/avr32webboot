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


#ifndef HTTP_POST_H_
#define HTTP_POST_H_

#include "webserver/httpd.h"

#define HTTP_POST_ACTION_NAME_MAX_LENGTH		20

typedef enum
{
	UNKNOWN = 0,
	FIRMWARE_UPDATE = 1,
	ACTION = 2,
} http_post_type;

/*
 * Note: These function headers are defined by httpd.h and are here just for documentation
 */

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
 *
 * err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
 *                        u16_t http_request_len, int content_len, char *response_uri,
 *                        u16_t response_uri_len, u8_t *post_auto_wnd)
 */

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
 *
 * err_t httpd_post_receive_data(void *connection, struct pbuf *p)
 */

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
 * void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
 */

/*************************************************************************************************
 * http_post_action_handler
 *
 * Called after a HTTP-Post request with the keyword "action" was received
 *
 * @param *buffer pointer to buffer which contains the HTTP-Post data
 * @param buffer_length length of buffer
 */
typedef void (*http_post_action_handler)(char *buffer, uint32_t buffer_length);

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
void http_post_register_handler(const char *action_name, http_post_action_handler function_pointer);

#endif /* HTTP_POST_H_ */