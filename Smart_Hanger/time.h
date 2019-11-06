/*
 * time.h
 *
 * Created: 2019-07-02 오전 10:28:01
 *  Author: bth8188
 */ 


#ifndef TIME_H_
#define TIME_H_

#include <stdio.h>
#include <avr/io.h>

void timer0init();
void incMilliSec();
uint32_t millis();
void incTime();
void incStop();
uint32_t stpmills();
void hh(char *curtime);
void mm(char *curtime);
void ss(char *curtime);
void timer1Init();






#endif /* TIME_H_ */
