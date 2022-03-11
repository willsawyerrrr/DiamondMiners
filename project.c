/*
 * project.c
 *
 * Main file for IN students
 *
 * Authors: Peter Sutton, Luke Kamols
 * Diamond Miners Inspiration: Daniel Cumming
 * Modified by William Sawyer
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"
#include "joystick.h"

#define JOYSTICK_LOWER_BOUND	-200
#define JOYSTICK_UPPER_BOUND	200

void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display is complete
	start_screen();
	
	// display(); // debugging joystick
	
	// Loop forever,
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	
	init_timer0();
	
	init_adc();
	
	// Turn on global interrupts
	sei();
}

void start_screen(void) {
	// Clear terminal screen and output a message
	clear_terminal();
	move_terminal_cursor(10,10);
	printf_P(PSTR("Diamond Miners"));
	move_terminal_cursor(10,12);
	printf_P(PSTR("CSSE2010 project by William Sawyer - 46963608"));
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	start_display();
	
	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1) {
		// First check for if a 's' is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's'
		char serial_input = -1;
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
		}
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S') {
			break;
		}
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED) {
			break;
		}
	}
}

void new_game(void) {
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the game and display
	initialise_game(0, 0);
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void) button_pushed();
	clear_serial_input_buffer();
}

void play_game(void) {
	uint32_t last_cursor_flash_time, last_detector_flash_time, last_bomb_flash_time, last_joystick_read_time, current_time;
	uint32_t bomb_planted_time = 0;
	uint32_t bomb_detonated_time = 0;
    uint32_t manhattan_time = 0;
	uint32_t pause_time = 0;
	uint32_t bomb_delay;
	int16_t joystick_x = 0;
	int16_t joystick_y = 0;
	uint8_t step_counter = 0;
	uint8_t paused = 0;
	uint8_t btn; //the button pushed
	uint8_t first_successful;
    char serial_input = -1;
	
	last_cursor_flash_time = get_current_time();
    last_detector_flash_time = get_current_time();
	last_joystick_read_time = get_current_time();
	
	// We play the game until it's over
	while (!is_game_over()) {
		while (!paused && !is_game_over()) {
			// We need to check if any button has been pushed, this will be
			// NO_BUTTON_PUSHED if no button has been pushed
			btn = button_pushed();
	
			if (serial_input_available()) {
				serial_input = fgetc(stdin);
	        }

			// check diagonal movement first
			if (joystick_x > JOYSTICK_UPPER_BOUND && joystick_y > JOYSTICK_UPPER_BOUND) { // up and right
				// first_successful allows us to try up and then right as well as right and then up
				first_successful = move_player(0, 1);
				step_counter += first_successful;
				step_counter += move_player(1, 0);
				if (!first_successful) {
					step_counter += move_player(0, 1);
				}
			} else if (joystick_x < JOYSTICK_LOWER_BOUND && joystick_y > JOYSTICK_UPPER_BOUND) { // up and left
				first_successful = move_player(0, 1);
				step_counter += first_successful;
				step_counter += move_player(-1, 0);
				if (!first_successful) {
					step_counter += move_player(0, 1);
				}
			} else if (joystick_x < JOYSTICK_LOWER_BOUND && joystick_y < JOYSTICK_LOWER_BOUND) { // down and left
				first_successful = move_player(0, -1);
				step_counter += first_successful;
				step_counter += move_player(-1, 0);
				if (!first_successful) {
					step_counter += move_player(0, -1);
				}
			} else if (joystick_x > JOYSTICK_UPPER_BOUND && joystick_y < JOYSTICK_LOWER_BOUND) { // down and right
				first_successful = move_player(0, -1);
				step_counter += first_successful;
				step_counter += move_player(1, 0);
				if (!first_successful) {
					step_counter += move_player(0, -1);
				}
			} else if (btn == BUTTON0_PUSHED || joystick_x > JOYSTICK_UPPER_BOUND
					|| serial_input == 'd' || serial_input == 'D') { // move right
				step_counter += move_player(1, 0);
			} else if (btn == BUTTON1_PUSHED || joystick_y < JOYSTICK_LOWER_BOUND
					|| serial_input == 's' || serial_input == 'S') { // move down
	            step_counter += move_player(0, -1);
			} else if (btn == BUTTON2_PUSHED || joystick_y > JOYSTICK_UPPER_BOUND
					|| serial_input == 'w' || serial_input == 'W') { // move up
	            step_counter += move_player(0, 1);
			} else if (btn == BUTTON3_PUSHED || joystick_x < JOYSTICK_LOWER_BOUND
					|| serial_input == 'a' || serial_input == 'A') { // move left
	            step_counter += move_player(-1, 0);
			} else if (serial_input == 'e'|| serial_input == 'E') {
	            inspect_facing();
			} else if (serial_input == 'c' || serial_input == 'C') {
	            toggle_cheat();
			} else if (serial_input == ' ') {
				if (plant_bomb()) {
					bomb_planted_time = get_current_time();
					bomb_delay = 350;
					last_bomb_flash_time = 0;
				}
			} else if (serial_input == 'p' || serial_input == 'P') {
				pause_time = pause_game();
				paused = 1;
			}
			
			serial_input = -1;
	
			current_time = get_current_time();
	
			if (current_time >= last_cursor_flash_time + 500) {
				// 500ms (0.5 second) has passed since the last time we
				// flashed the cursor, so flash the cursor
				flash_facing();
				
				// Update the most recent time the cursor was flashed
				last_cursor_flash_time = current_time;
			}
			
			manhattan_time = detect_diamond();
	
			if (manhattan_time == 0) {
				clear_detector();
			} else if (current_time >= last_detector_flash_time + manhattan_time) {
				flash_detector();
	
				// update the most recent time the detector was flashed
	            last_detector_flash_time = current_time;
			}

			if (bomb_planted_time) {
				danger_light(in_danger());
				if (current_time >= bomb_planted_time + BOMB_FUSE_TIME) {
					detonate_bomb();
					bomb_detonated_time = get_current_time();
					bomb_planted_time = 0;
					last_bomb_flash_time = 0;
				}
				if (!last_bomb_flash_time || current_time >= last_bomb_flash_time + bomb_delay) {
					if (flash_bomb()) {
						bomb_delay -= 75;
					}
					last_bomb_flash_time = get_current_time();
				}
			}

			if (bomb_detonated_time && current_time >= bomb_detonated_time + EXPLOSION_DELAY) {
				clear_explosion();
				bomb_detonated_time = 0;
			}

			joystick_x = 0;
			joystick_y = 0;
			
			if (current_time >= last_joystick_read_time + 200) {
				// 200ms has passed since we last read the joystick, so read it again
				joystick_x = read_joystick(1); // read joystick L/R at Pin A1
				joystick_y = read_joystick(0); // read joystick U/D at Pin A0
				last_joystick_read_time = get_current_time();
			}
	
			if (step_counter < 100) {
				seven_seg(step_counter);
			} else {
				seven_seg(99);
			}
		}
		
		while (paused && !is_game_over()) {
			if (serial_input_available()) {
				serial_input = fgetc(stdin);
			}
			if (serial_input == 'p' || serial_input == 'P') {
				unpause_game(pause_time);
				paused = 0;
			}
			serial_input = -1;
		}
	}
	// We get here if the game is over.
}

void handle_game_over() {
	uint32_t current_time;
	uint32_t last_game_over_time = 0;
	
	move_terminal_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10,15);
	printf_P(PSTR("Press a button to start again"));
	
	while (button_pushed() == NO_BUTTON_PUSHED) {
		current_time = get_current_time();
		
		if (current_time >= last_game_over_time + GAME_OVER_DELAY) {
			show_game_over();
			last_game_over_time = get_current_time();
		}
		
	}
}
