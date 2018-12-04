#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"
#include "project.h"

volatile int count = 0; // keeps track of current char
extern char sbuf[];     // buffer for serial reception
extern char sflag;      // flag signals new packet

void serial_init(unsigned short ubrr_value)
{

    // Set up USART0 registers
    UBRR0 = ubrr_value;
    UCSR0C = (3 << UCSZ00); // async, no parity
                            // 1 stop bit, 8 data bits
    UCSR0B |= (1 << TXEN0 | 1 << RXEN0);    // enable rx/tx

    // Enable tri-state
    DDRD |= (1 << PD3);
    PORTD &= ~(1 << PD3);
}

void serial_txchar(char ch)
{
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = ch;
}

void serial_stringout(char *s)
{
    int i = 0;
    // Call serial_txchar in loop to send a string
    while (s[i] != '\0') {
        serial_txchar(s[i]);
        i++;
    }
}

ISR(USART_RX_vect)
{

    // Handle received character
    char ch;

    ch = UDR0;   // Get the received charater

    // detect start of packet
    if(ch == '@') {
    	count = 0;
    }
    
    // Store in buffer
    if(count <= 6) {
	    sbuf[count] = ch;
    	count++;
    }
    // If message complete, set flag
    if(ch == '$') {
        sflag = 1;
    }
}