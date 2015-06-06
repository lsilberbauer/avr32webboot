/**************************************************************************************************
 *
 * @file log.c
 *
 * @brief Contains logging functions
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. Confidential proprietary
 *  Perfektastrasse 57/7, 1230 Wien, Austria. office@taurob.com
 *
 *************************************************************************************************/

#include "log.h"

/*************************************************************************************************
 * log_init
 *
 * Initializes the log, creates a DEBUG "Log started." message
 */
void log_init(void)
{
	// do nothing in bootloader
}

/*************************************************************************************************
 * log_report
 *
 * Creates a log entry
 *
 * @param level
 *
 *  taken from http://wiki.ros.org/Verbosity%20Levels:
 *
 *  DEBUG
 * 
 *  Information that you never need to see if the system is working properly. 
 *  Examples:
 *      "Received a message on topic X from caller Y"
 *      "Sent 20 bytes on socket 9".
 * 
 *  INFO
 * 
 *  Small amounts of information that may be useful to a user. 
 *  Examples:
 *      "Node initialized"
 *      "Advertised on topic X with message type Y"
 *      "New subscriber to topic X: Y"
 *
 *  WARN
 *
 *  Information that the user may find alarming, and may affect the output of the application, 
 *  but is part of the expected working of the system. 
 *  Examples:
 *      "Could not load configuration file from <path>. Using defaults."
 *
 *  ERROR
 *
 *  Something serious (but recoverable) has gone wrong. 
 *  Examples:
 *      "Haven't received an update on topic X for 10 seconds. Stopping robot until X continues broadcasting."
 *      "Received unexpected NaN value in transform X. Skipping..."
 *
 *  FATAL
 *
 *  Something unrecoverable has happened. 
 *  Examples:
 *      "Motors have caught fire!"
 *
 * @param module name of module
 * @param message
 * @param ... options for printf
 */
void log_report(log_verbosity_level level, const char *module, char *message, ...)
{
	// do nothing in bootloader
	asm ("nop");
}
