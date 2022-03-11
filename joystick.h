#ifndef JOYSTICK_H_
#define JOYSTICK_H_

void init_adc(void);

int16_t read_joystick(uint8_t pin);

void display(void);

#endif