/*
 * HX711.h
 *
 * Created: 2019-08-08 오후 2:43:33
 *  Author: bth8188
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#ifndef HX711_2_H_
#define HX711_2_H_

#define HX711_DDR2	DDRC
#define HX711_PORT2	PORTC
#define HX711_PIN2	PINC
#define HX711_DO_PIN2	PINC6
#define HX711_SCLK_PIN2	PINC7

void HX711_init2();
void power_off2();
void set_offset2(int _num);
uint32_t get_value2(int num);
void calc_value2(int _num, uint32_t weight[2]);





#endif /* HX711_H_ */
