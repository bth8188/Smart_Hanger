/*
 * time.c
 *
 * Created: 2019-07-02 오전 10:27:47
 *  Author: bth8188
 */ 

#include "time.h"

static uint32_t milliSec2 = 0;
static uint32_t milliSec = 0;
static uint8_t sec=0, min=0, hour=0;
static uint8_t mm3=0, sec2=0, min2=0, hour2=0;
static uint8_t _time[20] = {0,0,0,0,0};


void timer0init()
{
	//분주비 64
	TCCR0 |= _BV(CS02);
	//CTC mode 사용
	TCCR0 |= _BV(WGM01);
	//ouput compare interrupt enable
	TIMSK |= _BV(OCIE0);
	OCR0 = 250;
	// tcnt set 250
}

void incMilliSec()
{
	milliSec ++;
}

uint32_t millis()
{
	return milliSec;
}

void incTime()
{
	if((milliSec%1000) == 0)
	{
		sec++;
		if(sec >= 60)
		{
			sec = 0;
			min ++;
			if(min>=60)
			{
				min = 0;
				hour ++;
				if(hour>=24)
				{
					hour =0;
				}
			}
		}
	}
}

void hh(char *curtime)
{
	sprintf(curtime,"%02d",hour);
}

void mm(char *curtime)
{
	sprintf(curtime,"%02d",min);
}

void ss(char *curtime)
{
	sprintf(curtime,"%02d",sec);
}


void timer1Init()
{
	TCCR1B |= _BV(CS10) | _BV(CS11);
}
