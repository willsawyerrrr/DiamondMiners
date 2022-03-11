/*
 * display.c
 *
 * Author: Luke Kamols
 */ 

#include <stdio.h>
#include <avr/pgmspace.h>

#include "display.h"
#include "pixel_colour.h"
#include "ledmatrix.h"

// constant value used to display 'D <> M' on launch
static const uint8_t miners_display[MATRIX_NUM_COLUMNS] = 
		{125, 69, 69, 57, 0, 16, 56, 124, 56, 16, 0, 125, 33, 17, 33, 125};

void initialise_display(void) {
	// clear the LED matrix
	ledmatrix_clear();
}

void start_display(void) {
	PixelColour colour;
	MatrixColumn column_colour_data;
	uint8_t col_data;
		
	ledmatrix_clear(); // start by clearing the LED matrix
	for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++) {
		col_data = miners_display[col];
		// using the LSB as the colour determining bit, 1 is red, 0 is green
		if (col_data & 0x01) {
			colour = COLOUR_RED;
		} else {
			colour = COLOUR_GREEN;
		}
		// go through the top 7 bits (not the bottom one as that was our colour bit)
		// and set any to be the correct colour
		for(uint8_t i=7; i>=1; i--) {
			// If the relevant font bit is set, we make this a coloured pixel, else blank
			if(col_data & 0x80) {
				column_colour_data[i] = colour;
				} else {
				column_colour_data[i] = 0;
			}
			col_data <<= 1;
		}
		column_colour_data[0] = 0;
		ledmatrix_update_column(col, column_colour_data);
	}
}

void update_square_colour(uint8_t x, uint8_t y, uint8_t object) {
	// first check that this is a square within the game field
	// if outside the game field, don't update anything
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
		return;
	}
	
	// determine which colour corresponds to this object
	PixelColour colour;
	if (object == PLAYER) {
		colour = MATRIX_COLOUR_PLAYER;
	} else if (object == FACING) {
		colour = MATRIX_COLOUR_FACING;
	} else if (object == INSPECTED) {
		colour = MATRIX_COLOUR_BREAKABLE;
	} else if (object == BREAKABLE || object == UNBREAKABLE) {
		colour = MATRIX_COLOUR_WALL;
	} else if (object == DIAMOND) {
		colour = MATRIX_COLOUR_DIAMOND;
	} else if (object == UNDISCOVERED) {
		colour = MATRIX_COLOUR_UNDISCOVERED;
	} else if (object == BOMB) {
		colour = MATRIX_COLOUR_BOMB;
	} else if (object == EXPLOSION) {
		colour = MATRIX_COLOUR_EXPLOSION;
	} else {
		// anything unexpected (or empty) will be black
		colour = MATRIX_COLOUR_EMPTY;
	}

	// update the pixel at the given location with this colour
	ledmatrix_update_pixel(x, y, colour);
}