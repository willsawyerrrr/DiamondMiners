#define F_CPU 8000000L

#include <avr/io.h>

#include "terminalio.h"
#include "avr/pgmspace.h"
#include <stdio.h>

void init_adc(void) {
	ADMUX |= (1 << REFS0);
	ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // enable ADC, prescaler = 128
}

int16_t read_joystick(uint8_t pin) {
	pin &= 0b00000111;         // ANDing to limit input to 7
	ADMUX = (ADMUX & 0xF8) | pin;  // clear last 3 bits of ADMUX, OR with ch
	ADCSRA |= (1 << ADSC);        // start conversion
	while (ADCSRA & (1 << ADSC));    // wait until conversion is complete
	return ADC - 512;        // return ADC value
}

void display(void) {
	uint16_t x, y;
	while (1) {
		move_terminal_cursor(0, 0);
		x = read_joystick(1);
		y = read_joystick(0);
		printf_P(PSTR("x = %4d, y = %4d"), x, y);
	}
}