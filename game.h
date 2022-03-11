/*
** game.h
**
** Written by Luke Kamols, Daniel Cumming
**
** Function prototypes for those functions available externally
*/

#ifndef GAME_H_
#define GAME_H_

#include <inttypes.h>

#define BOMB_FUSE_TIME	2000
#define EXPLOSION_DELAY	500
#define GAME_OVER_DELAY	1000

/*
 * Initialise the game, creates the internal game state and updates
 * the display of this game
 */
void initialise_game(uint8_t level, uint8_t score);

/* returns which object is located at position (x,y)
 * the value returned will be EMPTY_SQUARE, BREAKABLE, UNBREAKABLE
 * or DIAMOND
 * if the given coordinates are out of bounds UNBREAKABLE will be returned
 */
uint8_t get_object_at(uint8_t x, uint8_t y);

/*
 * returns 1 if a given (x,y) coordinate is inside the bounds of 
 * the playing field, 0 if it is out of bounds
 */
uint8_t in_bounds(uint8_t x, uint8_t y);

/* update the player direction indicator display, by changing whether
 * it is visible or not, call this function at regular intervals to
 * have the indicator flash
 */
void flash_facing(void);

/*
 * move the position of the player by (dx, dy) if they can be moved
 * the player direction indicator should also be updated by this.
 * the player should move to square (x+dx, y+dy) if there is an
 * EMPTY_SQUARE or DIAMOND at that location.
 * get_object_at(x+dx, y+dy) can be used to check what is at that position
 * returns exit code 1 if move is valid
 * returns exit code 0 otherwise
 */
uint8_t move_player(int8_t dx, int8_t dy);

// returns 1 if the game is over, 0 otherwise
uint8_t is_game_over(void);

// updates colour of square containing direction indicator if there is a
// BREAKABLE at that location, does nothing otherwise
void inspect_facing(void);

void toggle_cheat(void);

// finds the nearest diamond to the player
// evaluates its Manhattan distance
// if distance is less than 4, flashes an LED
uint32_t detect_diamond();

// attempts to plant bomb at player's current location
// can plant bomb iff no other bomb is currently in play
// returns exit code 1 if planted new bomb
// returns exit code 0 otherwise
uint8_t plant_bomb(void);

// returns 1 if bomb is now displayed
// returns 0 otherwise
uint8_t flash_bomb(void);

void detonate_bomb(void);

void clear_explosion(void);

// returns 1 if the player is standing on or next to a bomb
// returns 0 otherwise
uint8_t in_danger(void);

// pauses the game, returns the current time
uint32_t pause_game(void);

// unpauses the game, sets current time to previous pause time
void unpause_game(uint32_t pause_time);

#endif

/*
 * How long? That is the question Robert Cancross posits as his focus returns, 
 * breaking the reverie he had been so graciously lost in. The dappled light 
 * filters through the leaves, falling softly upon his face. The shimmering 
 * warmth reminds him of Tuscany. Tuscany. How long had it been since he was 
 * home? He knew he could not provide an honest answer. And how long till he 
 * would return? ... Would he return? The weight of these questions was heavy 
 * upon him now, and this burden was more than he should like to carry. He 
 * busies himself by picking up equipment - with each item he lifts, the burden 
 * reduces a little more. There's no point waiting any longer, he has come this
 * far and waited this long. One last look towards the hills, before he turns 
 * towards the shaft of the mine, and begins his descent...
 */