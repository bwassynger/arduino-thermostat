#include "encoder.h"
#include "project.h"

#include <avr/io.h>
#include <avr/interrupt.h>

extern int ecount;	 // count global var
volatile int estate; // keeps track of state

void encoder_init() {
	// Using A3 and A4 as inputs

	// Enable pullups
	PORTC |= ((1 << PC3)|(1 << PC4));

	// Initialize interrupts
	// Enable portc interrupts
	PCICR |= (1 << PCIE1);
	// set specific bit interrupts
	PCMSK1 |= ((1 << PCINT11)|(1 << PCINT12));

	// sample inputs
	unsigned char sample = PINC;
	unsigned char a = sample & (1 << PC3);
	unsigned char b = sample & (1 << PC4);

	// get initial state
	if (!b && !a)
	estate = 0; // 00
    else if (!b && a)
	estate = 1; // 01
    else if (b && !a)
	estate = 2; // 10
    else
	estate = 3; // 11

}

// Encoder update interrupt
ISR(PCINT1_vect) {

	// Read the input bits and determine A and B
    unsigned char sample = PINC;
	unsigned char a = sample & (1 << PC3);
	unsigned char b = sample & (1 << PC4);
	int new_state = estate;

	// For each state, examine the two input bits to see if state
	// has changed, and if so set "new_state" to the new state,
	// and adjust the count value.
	if (estate == 0) {

	    // Handle A and B inputs for state 0
		if(a) {
			new_state = 1;
			ecount++;
		} else if(b) {
			new_state = 2;
			ecount--;
		}
	}
	else if (estate == 1) {

	    // Handle A and B inputs for state 1
		if(b) {
			new_state = 3;
			ecount++;
		} else if(!a) {
			new_state = 0;
			ecount--;
		}
	}
	else if (estate == 2) {

	    // Handle A and B inputs for state 2
		if(!b) {
			new_state = 0;
			ecount++;
		} else if(a) {
			new_state = 3;
			ecount--;
		}
	}
	else {   // estate = 3

	    // Handle A and B inputs for state 3
		if(!a) {
			new_state = 2;
			ecount++;
		} else if(!b) {
			new_state = 1;
			ecount--;
		}
	}
	
	// If state changed, update the value of old_state,
	// and set a flag that the state has changed.
	if (new_state != estate) {
	    estate = new_state;
	}
}