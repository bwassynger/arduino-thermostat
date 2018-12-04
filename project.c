/********************************************
 *
 *  Name:       Ben Wassynger
 *  Email:      wassynge@usc.edu
 *  Section:    31292
 *  Assignment: Lab 9 - Serial Communications
 *
 ********************************************/

/*
General project information

Button A: blue, low, A1 (PC1)
Button B: red, high, A2 (PC2)
*/


#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "lcd.h"
#include "encoder.h"
#include "serial.h"
#include "project.h"
#include "ds18b20.h"


#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0 register

int convertToF(short); // temperature conversion
int getTemp();
void updateAC(int temp, char low, char high);

int main(void) {

	// initialize LCD
    lcd_init();

    // initialize serial comms
    serial_init(MYUBRR);

    // Enable interrupts
    UCSR0B |= (1 << RXCIE0);    // Enable receiver interrupts

    sei();                      // Enable global interrupts

    // Setup inputs
    // enable pullups for buttons
    PORTC |= ((1 << PC1)|(1 << PC2));

    // setup LEDs
    // PB3 = D11 (A/C)
    // PB4 = D12 (Heat)
    DDRB |= ((1 << PB3)|(1 << PB4));

    // enable encoder
    encoder_init();

    // enable thermometer
    ds_init();

    // Show the splash screen
    lcd_writecommand(1);
    lcd_stringout("EE109 Project");
    lcd_moveto(1, 0);
    lcd_stringout("Ben Wassynger");
    _delay_ms(1000);
    lcd_writecommand(1);


    // char to store button inputs
    unsigned char sample;

    // buffer for lcd output
    char buf[17];

    // buffer for serial transmit
    char tbuf[7];

    // default threshold values
    char low = 40;
    char high = 100;

    // state variables
    int state = 0;
    int new_state = 0;

    // flag to update temp on lcd
    int update = 1;

    // local temp variabls
    int f;
    int newf;

    // remote temperature
    int rtemp = 0;

    // flag indicating data received
    extern char sflag;

    // encoder count var
    extern int ecount;

    // thermostat initialization
    // get temp and write to lcd
    f = getTemp();
    lcd_moveto(0,0);
    snprintf(buf, 16, "Temp: %d", f);
    lcd_stringout(buf);

    // read threshold values from eeprom
    int temp;
    temp = eeprom_read_byte((void *) 100);
    if(temp < 40 || temp > 100) {
    	// do nothing
    } else {
    	low = temp;
    }
    temp = eeprom_read_byte((void *) 101);
    if(temp < low || temp > 100) {
    	// do nothing
    } else  {
    	high = temp;
    }

    // display threshold values
    lcd_moveto(1,0);
    snprintf(buf, 16, "Low: %d Hi: %d", low, high);
    lcd_stringout(buf);

    // initial A/C and heat activation
    updateAC(f, low, high);

    // send initial temp over serial
    snprintf(tbuf, 7, "@%+d$", f);
    serial_stringout(tbuf);

    while (1) {     // Loop forever

        // temperature logic
        newf = getTemp();   // update temperature
        if(newf != f) {     // Temperature has changed
        	// send updated temp over serial
	        snprintf(tbuf, 7, "@%+d$", newf);
	        serial_stringout(tbuf);

	        // update LCD
        	snprintf(buf, 17, "Temp: %d Rmt:%d ", newf, rtemp);
	        lcd_moveto(0,0);
	        lcd_stringout(buf);
	        f = newf;

	        // update A/C and heat settings
	        updateAC(f, low, high);
        }
    	
    	// sample button inputs
        sample = PINC;
        if( !(sample & (1 << PC1)) ) {
            new_state = 1;
        } else if( !(sample & (1 << PC2)) ) {
            new_state = 2;
        }

        // state logic
        switch(state) {
            case 0: // Neither selected
            default:
                break;
            case 1: // Low selected
                if(ecount != low) {
                    update = 1;
                    if(ecount <= 40) {
                        ecount = 40;
                    } else if(ecount > high) {
                        ecount = high;
                    }
                    low = ecount;
                }
                if(update) {
		            update = 0;
		            lcd_moveto(1,0);
		            snprintf(buf, 17, "Low? %2d Hi: %d ", low, high);
		            lcd_stringout(buf);
		            updateAC(f, low, high);
		        }
            	if( !(sample & (1 << PC1)) ) { // return to default state
            		new_state = 0;
            		snprintf(buf, 17, "Low: %d Hi: %d ", low, high);
            		lcd_moveto(1,0);
            		lcd_stringout(buf);
            	}
                break;
            case 2: // High selected
                if(ecount != high) {
                    update = 1;
                    if(ecount <= low) {
                        ecount = low;
                    } else if(ecount > 100) {
                        ecount = 100;
                    }
                    high = ecount;
                }
                if(update) {
		            update = 0;
		            lcd_moveto(1,0);
		            snprintf(buf, 17, "Low: %2d Hi? %d ", low, high);
		            lcd_stringout(buf);
		            updateAC(f, low, high);
		        }
            	if( !(sample & (1 << PC2)) ) { // return to default state
            		new_state = 0;
            		snprintf(buf, 17, "Low: %d Hi: %d ", low, high);
            		lcd_moveto(1,0);
            		lcd_stringout(buf);
            	}
                break;
        }
        // update state logic
        if(new_state != state) {
            state = new_state;
            if(new_state == 1) {
                ecount = low;
            	lcd_moveto(1,0);
	            snprintf(buf, 17, "Low? %2d Hi: %d ", low, high);
	            lcd_stringout(buf);
            } else if(new_state == 2) {
                ecount = high;
            	lcd_moveto(1,0);
	            snprintf(buf, 17, "Low: %2d Hi? %d ", low, high);
	            lcd_stringout(buf);
            }
    		// save thresholds in eeprom
    		if( !(eeprom_read_byte((void*) 100) == low) ) {
    			eeprom_update_byte((void*) 100, low);
    		}
    		if( !(eeprom_read_byte((void*) 101) == high) ) {
    			// make sure not to write needlessly
        		eeprom_update_byte((void *) 101, high);
    		}
        }

        // serial receiver logic
        if(sflag) { // complete packet received
        	sscanf(sbuf, "%*c%d", &rtemp);
        	lcd_moveto(0,0);
        	snprintf(buf, 17, "Temp: %d Rmt:%d ", newf, rtemp);
        	lcd_stringout(buf);
        	sflag = 0;
        }

    }

    return 0 // never reached
}

// convert raw to fahrenheit
int convertToF(short raw) {
	return (int)raw * 9 / (5 * 16) + 32;
}

// get temp from sensor and convert
int getTemp() {
    unsigned char temp[2];
    signed short conv;
	ds_temp(temp);
	conv = ((temp[1] << 8) | temp[0]);
	return convertToF(conv);
}

// enable/disable heat and A/C
void updateAC(int temp, char low, char high) {
	if(temp < low) {
		// turn on heat
		PORTB |= (1 << PB4);
	} else {
		// turn off heat
		PORTB &= ~(1 << PB4);
	}

	if(temp > high) {
		// turn on AC
		PORTB |= (1 << PB3);
	} else {
		// turn off AC
		PORTB &= ~(1 << PB3);
	}
}