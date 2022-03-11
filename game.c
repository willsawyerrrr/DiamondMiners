/*
 * game.c
 *
 * Contains functions relating to the play of Diamond Miners
 *
 * Author: Luke Kamols
 */ 


#include "game.h"
#include "display.h"
#include "terminalio.h"
#include "avr\pgmspace.h"
#include "timer0.h"

#include <stdlib.h>
#include <stdio.h>

#define PLAYER_START_X  0
#define PLAYER_START_Y  0
#define FACING_START_X  1
#define FACING_START_Y  0
#define CHEAT_START     0

// game level layouts
// the values 0, 3, 4 and 5 are defined in display.h
// note that this is not laid out in such a way that level_n_layout[x][y]
// does not correspond to an (x,y) coordinate but is a better visual
// representation
static const uint8_t level_1_layout[HEIGHT][WIDTH] = 
		{
			{0, 3, 0, 3, 0, 0, 0, 4, 4, 0, 0, 4, 0, 4, 0, 4},
			{0, 4, 0, 4, 0, 0, 0, 3, 4, 4, 3, 4, 0, 3, 0, 4},
			{0, 4, 0, 4, 4, 4, 4, 0, 3, 0, 0, 0, 0, 4, 0, 4},
			{5, 4, 0, 4, 0, 0, 3, 0, 0, 4, 0, 0, 0, 4, 0, 10},
			{4, 4, 3, 4, 5, 0, 4, 0, 0, 4, 3, 4, 0, 0, 4, 4},
			{0, 0, 0, 4, 4, 4, 4, 0, 4, 0, 0, 0, 4, 3, 0, 4},
			{0, 0, 0, 3, 0, 0, 3, 0, 3, 0, 3, 0, 3, 0, 0, 4},
			{0, 0, 0, 4, 0, 0, 3, 0, 4, 0, 0, 3, 3, 0, 5, 4} 
		};
#define NUM_L1_DIAMONDS 3
static const uint8_t l1_diamonds[NUM_L1_DIAMONDS][2] = {{0, 4}, {4, 3}, {14, 0}};

static const uint8_t level_2_layout[HEIGHT][WIDTH] =
		{
			{4, 4, 3, 4, 4, 0, 4, 4, 3, 3, 3, 4, 0, 3, 0, 4},
			{0, 0, 0, 5, 3, 0, 0, 4, 4, 4, 0, 4, 5, 4, 0, 4},
			{3, 4, 4, 4, 4, 0, 3, 0, 0, 3, 0, 4, 0, 4, 0, 3},
			{0, 3, 0, 4, 0, 0, 0, 0, 0, 4, 0, 4, 0, 3, 0, 3},
			{4, 4, 4, 4, 0, 0, 4, 4, 3, 4, 0, 0, 4, 4, 4, 0},
			{0, 0, 0, 3, 0, 0, 4, 5, 0, 4, 0, 0, 0, 0, 3, 3},
			{0, 0, 0, 4, 3, 3, 0, 0, 0, 4, 0, 4, 4, 4, 4, 10},
			{0, 0, 0, 4, 3, 4, 4, 4, 4, 4, 0, 3, 5, 0, 4, 0},
		};
#define NUM_L2_DIAMONDS	4
static const uint8_t l2_diamonds[NUM_L2_DIAMONDS][2] = {{3, 7}, {7, 3}, {12, 0}, {12, 7}};
		
#define NUM_DIRECTIONS 8
static const uint8_t directions[NUM_DIRECTIONS][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}};

// variables for the current state of the game
uint8_t playing_field[WIDTH][HEIGHT]; // what is currently located at each square
uint8_t visible[WIDTH][HEIGHT]; // whether each square is currently visible
uint8_t player_x, player_y;
uint8_t facing_x, facing_y, facing_visible;
uint8_t bomb_x, bomb_y, bomb_planted, bomb_visible, det_x, det_y;
uint8_t cheating;
uint8_t level;
uint8_t total_score = 0;
uint8_t score;
uint8_t diamonds_available;
uint8_t game_over;

// function prototypes for this file
void discoverable_dfs(uint8_t x, uint8_t y);
void initialise_terminal_display(void);
void initialise_game_display(void);
void initialise_game_state(uint8_t level, uint8_t score);
void collect_diamond(uint8_t x, uint8_t y);
void finish_level(void);

/*
 * initialise the game state, sets up the playing field, visibility
 * the player and the player direction indicator
 */
void initialise_game_state(uint8_t current_level, uint8_t current_score) {
	// initialise the player position and the facing position
	player_x = PLAYER_START_X;
	player_y = PLAYER_START_Y;
	facing_x = FACING_START_X;
	facing_y = FACING_START_Y;
	facing_visible = 1;
	bomb_planted = 0;
    cheating = CHEAT_START;
	level = current_level + 1;
	total_score += current_score;
	score = 0;
	if (level % 2 == 0) {
		diamonds_available = NUM_L2_DIAMONDS;
		} else {
		diamonds_available = NUM_L1_DIAMONDS;
	}
	game_over = 0;
	
	// go through and initialise the state of the playing_field
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			// initialise this square based on the starting layout
			// the indices here are to ensure the starting layout
			// could be easily visualised when declared
			if (level % 2 == 1) {
				playing_field[x][y] = level_1_layout[HEIGHT - 1 - y][x];
			} else if (level % 2 == 0) {
				playing_field[x][y] = level_2_layout[HEIGHT - 1 -y][x];
			}
			// set all squares to start not visible, this will be
			// updated once the display is initialised as well
			visible[x][y] = 0;
		}
	}	
}

/*
 * initialise the display of the game, shows the player and the player
 * direction indicator. 
 * executes a visibility search from the player's starting location
 */
void initialise_game_display(void) {
	// initialise the display
	initialise_display();
	// make the entire playing field undiscovered to start
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			update_square_colour(x, y, UNDISCOVERED);
		}
	}
	// now explore visibility from the starting location
	discoverable_dfs(player_x, player_y);
	// make the player and facing square visible
	update_square_colour(player_x, player_y, PLAYER);
	update_square_colour(facing_x, facing_y, FACING);
}

void initialise_terminal_display(void) {
	clear_terminal();
	move_terminal_cursor(LEVEL_X, LEVEL_Y);
	printf_P(PSTR("Level: %d"), level);
	move_terminal_cursor(SCORE_X, SCORE_Y);
	printf_P(PSTR("Diamonds Collected: %d of %d"), score, diamonds_available);
	move_terminal_cursor(CHEAT_X, CHEAT_Y);
	printf_P(PSTR("Cheat Mode: Disabled"));
	move_terminal_cursor(0, 0); // gets cursor out of the way
}

void initialise_game(uint8_t level, uint8_t score) {
	// to initialise the game, we need to initialise the state (variables),
	// the display and the terminal
	initialise_game_state(level, score);
	initialise_game_display();
	initialise_terminal_display();
}

uint8_t in_bounds(uint8_t x, uint8_t y) {
	// a square is in bounds if 0 <= x < WIDTH && 0 <= y < HEIGHT
	return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

uint8_t get_object_at(uint8_t x, uint8_t y) {
	// check the bounds, anything outside the bounds
	// will be considered an unbreakable wall
	if (!in_bounds(x,y)) {
		return UNBREAKABLE;
	} else {
		//if in the bounds, just index into the array
		return playing_field[x][y];
	}
}

void flash_facing(void) {
	// only flash the facing cursor if it is in bounds
	if (in_bounds(facing_x, facing_y)) {
		if (facing_visible) {
			// we need to flash the facing cursor off, it should be replaced by
			// the colour of the piece which is at that location
			uint8_t piece_at_cursor = get_object_at(facing_x, facing_y);
			update_square_colour(facing_x, facing_y, piece_at_cursor);
		
		} else {
			// we need to flash the facing cursor on
			update_square_colour(facing_x, facing_y, FACING);
		}
		facing_visible = 1 - facing_visible;
	}
}

uint8_t move_player(int8_t dx, int8_t dy) {
	uint8_t valid = 0;
	// remove the display of the player at the current location
	// and the player direction indicator and replace them each
	// with whatever else is at those locations
    update_square_colour(player_x, player_y, get_object_at(player_x, player_y));
	update_square_colour(facing_x, facing_y, get_object_at(facing_x, facing_y));
	
	if (get_object_at(player_x, player_y) == EXIT && dx == 1 && dy == 0) {
		if (level % 2 == 1 && score == 3) {
			finish_level();
		} else if (level % 2 == 0 && score == 4) {
			finish_level();
		}
	}

	// if the player can move, update the position of the player
    uint8_t dest_object = get_object_at(player_x + dx, player_y + dy);
    if (dest_object == DIAMOND || dest_object == EMPTY_SQUARE
			|| dest_object == BOMB || dest_object == EXIT) {
        player_x += dx;
        player_y += dy;
		valid = 1;
    }

    // update direction indicator
    facing_x = player_x + dx;
	facing_y = player_y + dy;

    // display the player at the new location
    update_square_colour(player_x, player_y, PLAYER);
	
	if (get_object_at(player_x, player_y) == DIAMOND) {
		collect_diamond(player_x, player_y);
	}

    // restart player direction indicator flashing cycle
    facing_visible = 1;
    flash_facing();
	
	return valid;
}

void inspect_facing(void) {
	uint8_t inspected = get_object_at(facing_x, facing_y);
    if (cheating) {
        if (inspected == BREAKABLE || inspected == INSPECTED) {
			playing_field[facing_x][facing_y] = EMPTY_SQUARE;
			discoverable_dfs(facing_x, facing_y);
        }
	} else {
		if (inspected == BREAKABLE) {
			playing_field[facing_x][facing_y] = INSPECTED;
			visible[facing_x][facing_y] = 1;
			update_square_colour(facing_x, facing_y, INSPECTED);
        }
    }
}

void toggle_cheat(void) {
    cheating = 1 - cheating;
	update_cheat(cheating);
}

// checks if the player is on a diamond. If they are, remove the
// diamond, increment their score and update terminal "scoreboard"
void collect_diamond(uint8_t x, uint8_t y) {
    playing_field[x][y] = EMPTY_SQUARE;
    score++;
    update_score(score);
}

uint32_t detect_diamond() {
    uint8_t diamond_x, diamond_y, x_diff, y_diff, distance, shortest;
	shortest = 5;

	if (level % 2 == 1) {
		for (int i = 0; i < NUM_L1_DIAMONDS; i++) {
			diamond_x = l1_diamonds[i][0];
			diamond_y = l1_diamonds[i][1];
			if (get_object_at(diamond_x, diamond_y) == DIAMOND) {
				x_diff = abs(player_x - diamond_x);
				y_diff = abs(player_y - diamond_y);
				distance = x_diff + y_diff;
				if (distance < shortest) {
					shortest = distance;
				}
			}
		}
	} else if (level % 2 == 0) {
		for (int i = 0; i < NUM_L2_DIAMONDS; i++) {
			diamond_x = l2_diamonds[i][0];
			diamond_y = l2_diamonds[i][1];
			if (get_object_at(diamond_x, diamond_y) == DIAMOND) {
				x_diff = abs(player_x - diamond_x);
				y_diff = abs(player_y - diamond_y);
				distance = x_diff + y_diff;
				if (distance < shortest) {
					shortest = distance;
				}
			}
		}
	}

	if (shortest == 4) {
        return 750;
    } else if (shortest == 3) {
        return 500;
    } else if (shortest == 2) {
        return 250;
    } else if (shortest == 1) {
        return 125;
    }
	
	return 0;
}

uint8_t plant_bomb(void) {
	if (!bomb_planted) {
		bomb_x = player_x;
		bomb_y = player_y;
		playing_field[bomb_x][bomb_y] = BOMB;
		bomb_planted = 1;
		bomb_visible = 1;
		return 1;
	}
	return 0;
}

uint8_t flash_bomb(void) {
	if (bomb_visible) {
		update_square_colour(bomb_x, bomb_y, EMPTY_SQUARE);
	} else {
		update_square_colour(bomb_x, bomb_y, BOMB);
	}
	bomb_visible = 1 - bomb_visible;
	return bomb_visible;
}

void detonate_bomb(void) {
	uint8_t x_adj, y_adj, exploded;
	playing_field[bomb_x][bomb_y] = EMPTY_SQUARE;
	update_square_colour(bomb_x, bomb_y, EMPTY_SQUARE);
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		x_adj = bomb_x + directions[i][0];
		y_adj = bomb_y + directions[i][1];
		// if this square is in bounds, it should be exploded
		if (in_bounds(x_adj, y_adj)) {
			exploded = get_object_at(x_adj, y_adj);
			if (exploded == BREAKABLE || exploded == INSPECTED
					|| exploded == EXIT) {
				playing_field[x_adj][y_adj] = EMPTY_SQUARE;
				discoverable_dfs(x_adj, y_adj);
			}
			update_square_colour(x_adj, y_adj, EXPLOSION);
		}
	}
	if (in_danger()) {
		game_over = 1;
	}
	bomb_planted = 0;
	det_x = bomb_x;
	det_y = bomb_y;
}

void clear_explosion() {
	uint8_t x_adj, y_adj;
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		x_adj = det_x + directions[i][0];
		y_adj = det_y + directions[i][1];
		// if this square is in bounds, it should be exploded
		if (in_bounds(x_adj, y_adj)) {
			update_square_colour(x_adj, y_adj, get_object_at(x_adj, y_adj));
		}
	}
}

uint8_t in_danger(void) {
	uint8_t x_adj, y_adj;
	if (bomb_x == player_x && bomb_y == player_y) {
		return 1;
	}
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		x_adj = bomb_x + directions[i][0];
		y_adj = bomb_y + directions[i][1];
		// if the player is next to the bomb, they are in danger
		if (x_adj == player_x && y_adj == player_y) {
			return 1;
		}
	}
	return 0;
}

uint32_t pause_game(void) {
	move_terminal_cursor(PAUSED_X, PAUSED_Y);
	printf_P(PSTR("Game paused"));
	move_terminal_cursor(0, 0);
	return get_current_time();
}

void unpause_game(uint32_t pause_time) {
	move_terminal_cursor(PAUSED_X, PAUSED_Y);
	clear_to_end_of_line();
	move_terminal_cursor(0, 0);
	set_current_time(pause_time);
}

void finish_level(void) {
	initialise_game(level, score);
}

uint8_t is_game_over(void) {
	// initially the game never ends
	return game_over;
}

/*
 * given an (x,y) coordinate, perform a depth first search to make any
 * squares reachable from here visible. If a wall is broken at a position
 * (x,y), this function should be called with coordinates (x,y)
 * YOU SHOULD NOT NEED TO MODIFY THIS FUNCTION
 */
void discoverable_dfs(uint8_t x, uint8_t y) {
	uint8_t x_adj, y_adj, object_here;
	// set the current square to be visible and update display
	visible[x][y] = 1;
	object_here = get_object_at(x, y);
	update_square_colour(x, y, object_here);
	// we can continue exploring from this square if it is empty
	if (object_here == EMPTY_SQUARE || object_here == DIAMOND || object_here == EXIT) {
		// consider all 4 adjacent square
		for (int i = 0; i < NUM_DIRECTIONS; i++) {
			x_adj = x + directions[i][0];
			y_adj = y + directions[i][1];
			// if this square is not visible yet, it should be explored
			if (in_bounds(x_adj, y_adj) && !visible[x_adj][y_adj]) {
				// this recursive call implements a depth first search
				// the visible array ensures termination
				discoverable_dfs(x_adj, y_adj);
			}
		}
	}
}