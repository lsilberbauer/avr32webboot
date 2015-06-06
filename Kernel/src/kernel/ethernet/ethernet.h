/**************************************************************************************************
 *
 * @file ethernet.h
 *
 * @brief Initializes the LwIP Stack and starts all Ethernet-related tasks
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#ifndef ETHERNET_H_
#define ETHERNET_H_

/*************************************************************************************************
 * ethernet_init
 *
 * Initializes the LwIP stack and starts all Ethernet-related tasks
 */
void ethernet_init(void);


#endif  /* ETHERNET_H_ */
