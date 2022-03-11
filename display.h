/*
 * display.h
 *
 * Author: Luke Kamols
 */ 

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "pixel_colour.h"

// display dimensions, these match the size of the playing field
#define WIDTH  16
#define HEIGHT 8

// object definitions
#define EMPTY_SQUARE	0
#define PLAYER			1
#define FACING			2
#define BREAKABLE	 	3
#define UNBREAKABLE		4
#define DIAMOND			5
#define UNDISCOVERED	6
#define INSPECTED		7
#define BOMB			8
#define EXPLOSION		9
#define EXIT			10

// matrix colour definitions
#define MATRIX_COLOUR_EMPTY			COLOUR_BLACK
#define MATRIX_COLOUR_PLAYER		COLOUR_RED
#define MATRIX_COLOUR_FACING		COLOUR_LIGHT_RED
#define MATRIX_COLOUR_BREAKABLE     COLOUR_LIGHT_GREEN
#define MATRIX_COLOUR_WALL			COLOUR_YELLOW
#define MATRIX_COLOUR_DIAMOND		COLOUR_GREEN
#define MATRIX_COLOUR_UNDISCOVERED	COLOUR_LIGHT_YELLOW
#define MATRIX_COLOUR_BOMB			COLOUR_ORANGE
#define MATRIX_COLOUR_EXPLOSION		COLOUR_LIGHT_ORANGE

/*
 * initialise the display for the playing field
 */
void initialise_display(void);

/*
 * display a start screen
 */
void start_display(void);

/*
 * updates the colour at square (x, y) to be the colour
 * of the object 'object'
 * 'object' is expected to be EMPTY_SQUARE, PLAYER, FACING, 
 * BREAKABLE, UNBREAKABLE, DIAMOND or UNDISCOVERED
 */
void update_square_colour(uint8_t x, uint8_t y, uint8_t object);

#endif 