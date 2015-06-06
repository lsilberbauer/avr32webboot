/**************************************************************************************************
 *
 * @file ssi.c
 *
 * @brief Initializes the Server Side Includes (SSI) for the creation of dynamic webpages
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#include "ssi/ssi.h"
#include "webserver/httpd.h"
#include <string.h>

#include "log/log.h"

static const char *module_name = "ssi";

static char **tags;
static ssi_handler *handlers;
static uint32_t number_of_tags = 0;

/*************************************************************************************************
 * ssi_handle_tags
 *
 * Identifies the tag and creates the dynamic output
 *
 * @param iIndex provides the zero-based index of the tag as
 *               found in the ppcTags array and identifies the tag that is to be processed
 * @param *pcInsert pointer to output buffer
 * @param iInsertLen length of output buffer
 * @param current_tag_part number of current tag
 * @param *next_tag_part pointer to next number of tag, or 0xFFFF to stop
 *
 * @return number of characters written to the output buffer
 */
static u16_t ssi_handle_tags(int iIndex,
                             char *pcInsert,
							 int iInsertLen,
							 u16_t current_tag_part,
							 u16_t *next_tag_part)
{
	uint32_t total_length = 0;

	if (iIndex < number_of_tags)
	{
		/* call previously registered handling function and pass her the parameters */
		total_length = handlers[iIndex](pcInsert, iInsertLen, current_tag_part, next_tag_part);
	}

	return total_length;
}

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
void ssi_register_handler(const char *keyword, ssi_handler function_pointer)
{
	if (strlen(keyword) > SSI_KEYWORD_MAX_LENGTH)
	{
		log_report(UECU_LOG_ERROR, module_name, "tag %s too long, truncated", keyword);
	}

	/* Make tag array bigger to fit the new keyword
	 * Note: see here http://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc
	 */
	tags = pvPortRealloc(tags, (number_of_tags + 1) * sizeof(char *));

	if (tags != NULL)
	{
		tags[number_of_tags] = keyword;
	}

	handlers = pvPortRealloc(handlers, (number_of_tags + 1) * sizeof(handlers));

	if (handlers != NULL)
	{
		handlers[number_of_tags] = function_pointer;
	}

	number_of_tags++;
}

/*************************************************************************************************
 * ssi_init
 *
 * Initializes the Server Side Includes (SSI) for the creation of dynamic webpages
 */
void ssi_init(void)
{
	http_set_ssi_handler(&ssi_handle_tags, tags, number_of_tags);
}
