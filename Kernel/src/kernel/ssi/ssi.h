/**************************************************************************************************
 *
 * @file ssi.h
 *
 * @brief Initializes the Server Side Includes (SSI) for the creation of dynamic webpages
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#ifndef SSI_H_
#define SSI_H_

#include <stdint.h>

#define SSI_KEYWORD_MAX_LENGTH			20

/*************************************************************************************************
 * ssi_handler
 *
 * Creates the dynamic output for one specific tag, has to be registered through
 * ssi_register_handler() before calling ssi_init().
 *
 * @param *buffer pointer to output buffer
 * @param buffer_length length of output buffer
 * @param current_tag_part number of current tag
 * @param *next_tag_part pointer to next number of tag, or 0xFFFF to stop
 *
 * @return number of characters written to the output buffer
 */
typedef uint16_t (*ssi_handler)(char *buffer, uint32_t buffer_length, uint16_t current_tag_part, uint16_t *next_tag_part);

/*************************************************************************************************
 * ssi_register_handler
 *
 * Registers a function to be called when "keyword" is parsed by the http-server. This handler-
 * function has to provide the HTML or JavaScript text which is inserted after the keyword tag.
 *
 * Example for keyword tag in HTML: <!--#registry_content--> (where "registry_content" is the keyword)
 *
 * @param keyword, max length: 20 characters including "\0" termination, case sensitive!
 * @param function_pointer
 */
void ssi_register_handler(const char *keyword, ssi_handler function_pointer);

/*************************************************************************************************
 * ssi_init
 *
 * Initializes the Server Side Includes (SSI) for the creation of dynamic webpages, tells the http
 * server which ssi_handlers have been registered
 *
 * @pre: ssi_register_handler has to be called BEFORE calling ssi_init
 */
void ssi_init(void);


#endif  /* SSI_H_ */
