# avr32webboot

HTML5 webinterface and bootloader for AVR32

## Release V1.0

Allows to flash an AVR32 via a nice looking HTML5 webinterface. 
Simply upload a .srec file, test the new firmware and apply or reject changes.
Uses FreeRTOS, LwIP, ASF and Miniport.

Contributions and ports welcome!

## Features

* Runs on an EVK1100 Development Board
* FreeRTOS V8.2.0
* LwIP 1.4.1 + httpserver_raw app + SSI + POST support
* .srec File Parser (checks parity and start address)
* Automatically creates a backup of the running program in AT45DBX external flash memory
* Implements a simply registry: Change parameters like IP address via the webinterface and tune your application on-the-fly
* Easy to use SSI (Server Side Includes) and HTTP-POST handling functions
* Application stub for easy extensibility

## Requirements

* AVR Studio 6.2 with ASF (Atmel Software Framework)
* Atmel EVK1100 Demo Board
* JTAG programmer
* Ethernet cable from your PC to the Demo Board
* Recommended: Internet connection of your PC (via a second LAN adapter or WiFi) for HTML5 features
* Optional: A serial cable connected to UART1 with 115200 baud, 8N1 to display debug info

## Getting started

1. Download avr32webboot.hex from http://www.taurob.com/U-ECU/binaries/ and flash it on your EVK1100. 
2. Press the PB0 button on startup to initialize the EEPROM for the first time.
3. go to step 4.

OR

1. Go to and load the Bootloader project in this Solution, unload the Kernel project, build and download the Bootloader to your EVK1100 (or press F5).
2. Go to the Kernel project in this Solution, load it and unload the Bootloader, build the Kernel and download it also. Do not change the project settings.
3. Press the PB0 button on startup to initialize the EEPROM for the first time.

4. Give your computer an IP address in the 10.0.0.0/24 range, e.g. 10.0.0.100, subnetmask 255.255.255.0
5. Ping 10.0.0.50 (that's your EVK1100)
6. Enter 10.0.0.50 in your browser's address field
7. Scroll down to "Registry". Change "Task 1 Frequency" to 50. The LEDs should flash faster now :-)
8. In AVR Studio, open the file src/application/application.h. Change the define APPLICATION_NAME to anything you like, e.g. "My Coolest Application Ever".
9. Rebuild the project (F7)
10. In your browser, scroll down to Firmware Update
11. Select the newly build .srec file under Kernel\Debug
12. Press the upload button in the lower half of the button (there is still a bug in CSS)
13. Wait until the web page reloads. Check the top of the page - the application name should have changed now.
14. Keep or reject the new firmware image as you like - if you don't press "Apply Settings" the previous firmware image will be automatically restored upon reset.
15. Go to src/application/application.c and put in your own code 

## Known Bugs

https://github.com/lsilberbauer/avr32webboot/issues

## Questions

https://groups.google.com/d/forum/avr32webboot

## License

See LICENSE.txt

## Credits

http://www.freertos.org/
http://savannah.nongnu.org/projects/lwip/
http://www.atmel.com/tools/avrsoftwareframework.aspx?tab=overview
http://html5up.net/miniport


written on June 6, 2015 by Lukas Silberbauer


(c) 2015 taurob GmbH