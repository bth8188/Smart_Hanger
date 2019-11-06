/*
 * button.h
 *
 * Created: 2019-07-12 오전 10:22:06
 *  Author: bth8188
 */ 


#ifndef BUTTON_H_
#define BUTTON_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BUTTON_DDR DDRG
#define BUTTON_PORT PORTG
#define BUTTON_PIN PING
#define start_stop_button PING1

void Button_Init(int _pin);
int button(uint8_t _num);



#endif /* BUTTON_H_ */
