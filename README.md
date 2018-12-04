# arduino-thermostat
This codebase was designed for my final project of EE109, Introduction to Embedded Systems. It is designed for an Arduino UNO Microcontroller connected to a DS18B20 temperature sensor. The software allows low and high temperatures to be set and saved to the flash storage on the arduino using a rotary encoder, and the system will turn on and off A/C and Heating systems (symbolized by LEDs in my project). The arduino displays current settings and temperature on an LCD shield, and it can communicate over a serial connection to other arduinos running the same software to display temperatures from remote systems.

### project.c
Contains the main running loop of the software. 

### encoder.c
Receives interrupts from the rotary encoder and updates threshold values from the main program. Used to update high/low temperature thresholds for A/C and heating activation.

### serial.c
Sends and receives temperature over an RS-232 serial connection to another arduino system. Uses a packet system to prevent data corruption. Start of packets are indicated by the '@' character, and will be followed by a + or - sign to indicate the sign of the temperature. Up to three ASCII digits can be sent following which represent the temperature in degrees Fahrenheit as an integer. After all characters have been sent, the end of the packet is indicated by the '$' character.
For example, if the current temperature is 68 degrees fahrenheit, the packet will be the string: "@+68$".
Packet reception is also contained in this file but are decoded in the main program.

### ds18b20.c
Contains interface routines for the DS18B20 temperature sensor. The sensor is initialized and temperature can be updated through the routines in this file. 

### lcd.c
Contains the routines necessary to interface with the LCD shield on the arduino.

### Makefile
Contains info necessary to build and flash the project to an arduino.
