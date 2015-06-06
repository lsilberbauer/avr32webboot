# avr32webboot

Release V1.0
------------

Allows to flash an AVR32 by a nice looking HTML5 webinterface. 
Simply upload a .srec file, test the new firmware and apply or reject changes.
Uses FreeRTOS, LwIP, ASF and Miniport.

Contributions and ports welcome!

Features
--------
* Runs on an EVK1100 Development Board
* FreeRTOS V8.2.0
* LwIP 1.4.1 + httpserver_raw app + SSI + POST support
* .srec File Parser (checks parity and start address)
* Automatically creates a backup of the running programm in AT45DBX external flash memory
* Implements a simply registry: Change parameters like IP address via the webinterface and tune your application on-the-fly
* Easy to use SSI (Server Side Includes) and HTTP-POST handling functions
* Application stub for easy extensibility

Getting started
---------------

1.) Download avr32webboot.hex from the WIKI and download it to your EVK1100. 
2.) Press the PB0 button on startup to initialize the EEPROM for the first time.
3.) go to step 4.)

OR

1.) Go to and load the Bootloader project in this Solution, unload the Kernel project, build and download the Bootloader to your EVK1100. 
2.) Go to the Kernel project in this Solution, load it and unload the Bootloader, build the Kernel and download it also. Do not change the project settings.
3.) Press the PB0 button on startup to initialize the EEPROM for the first time.

4.) Give your computer an IP address in the 10.0.0.0/24 range, e.g. 10.0.0.100, subnetmask 255.255.255.0
5.) Ping 10.0.0.50 (that's your EVK1100)
6.) Enter 10.0.0.50 in your browser's address field
7.) Rest should be self-explanatory

Questions:
https://groups.google.com/d/forum/avr32webboot
or 
avr32webboot@googlegroups.com

License: See LICENSE.txt
Author: Lukas Silberbauer
(c) 2015 taurob GmbH