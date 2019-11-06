/*
 * DHT.c
 *
 * Created: 2019-08-01 오후 9:01:57
 *  Author: bth8188
 */ 
#include "DHT.h"

enum {OK, Fail_H, Fail_T, checksum, Timeout} DHT_status;

uint8_t DHT_type;

uint8_t get_DHT_status()
{
	return DHT_status;
}

uint8_t check_timeout(uint8_t *_checker, uint8_t _timer)
{
	_delay_us(2);
	(*_checker) ++;
	if(*_checker > _timer)
	{
		DHT_status = Timeout;
		return 1;
	}
	return 0;
}

void send_s_signal(uint8_t _pin)
{
	DHT_DDR |= (0x01 << (DHT_DATA_PIN + _pin));		// DDR 출력 설정
	DHT_PORT |= (0x01 << (DHT_DATA_PIN + _pin));		// high 신호 송출
	DHT_PORT &= ~(0x01 << (DHT_DATA_PIN + _pin));	// low 신호 송출
	_delay_ms(20);		// 최소 18ms이상의 대기시간
	DHT_PORT |= (0x01 << (DHT_DATA_PIN + _pin));		// high 신호 송출
}

void receive_signal(uint8_t _pin)
{
	uint8_t checker = 0;
	DHT_DDR &= ~(0x01 << (DHT_DATA_PIN + _pin));
	while((DHT_PIN & (0x01 << (DHT_DATA_PIN + _pin))))		//DHT 신호가 0이 되는걸 대기
	{
		if(check_timeout(&checker, 30)) break;
	}
	checker = 0;
	if(DHT_status == OK)
	{
		while(!(DHT_PIN&(0x01 << (DHT_DATA_PIN + _pin))))		//DHT 신호가 1이 되는걸 대기
		{
			if(check_timeout(&checker, 50)) break;
		}
		checker = 0;
		while((DHT_PIN & (0x01 << (DHT_DATA_PIN + _pin))))	//DHT 신호가 0이 되는걸 대기
		{
			if(check_timeout(&checker, 50)) break;
		}
		checker = 0;
	}
}

void receive_raw_data(uint8_t _data[4] ,uint8_t _pin, uint8_t _type)
{
	uint8_t checker = 0, raw_data[5];
	DHT_type = _type;
	int conH =0, conV =0;
	cli();
	DHT_status = OK;
	send_s_signal(_pin);
	receive_signal(_pin);
	if(DHT_status == OK)
	{
		for(int i=0; i< 5; i++)
		{
			raw_data[i] = 0;
			for(int j =7; j> -1; j--)
			{
				while(!(DHT_PIN&(0x01 << (DHT_DATA_PIN + _pin))))		//신호가 1이 되는걸 대기
				{
					if(check_timeout(&checker, 20))
					{
						i = 5;
						break;
					}
				}
				checker = 0;
				if(DHT_status == OK)
				{
					_delay_us(35);
					raw_data[i] |= (DHT_PIN&(0x01 << (DHT_DATA_PIN + _pin))) == (0x01 << (DHT_DATA_PIN + _pin))? (1 << j) : 0;
					while((DHT_PIN&(0x01 << (DHT_DATA_PIN + _pin))))
					{
						if(check_timeout(&checker, 25))
						{
							i = 5;
							break;
						}
					}
					checker =0;
				}
			}
		}
	}
	DHT_DDR |= (0x01 << (DHT_DATA_PIN + _pin));		// DDR 출력 설정
	DHT_PORT |= (0x01 << (DHT_DATA_PIN + _pin));		// high 신호 송출
	sei();
 	if(DHT_status == OK)
 	{
		if ((DHT_type == _DHT11))
	 	if(raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3] != raw_data[4]) DHT_status = checksum;
		 
		else if (DHT_type == _DHT22)
		{
			if(raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3] > 0xff)
			{
				if(raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3] - 0x100 != raw_data[4]) DHT_status = checksum;
			}
			else if(raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3] != raw_data[4]) DHT_status = checksum;
		}
		 
		if (DHT_type == _DHT11)
		{
			if((raw_data[0] >95) || (raw_data[0]) < 15) DHT_status = Fail_H;
			if((raw_data[2] >52) || (raw_data[2]) < -2) DHT_status = Fail_T;
		}
		
		
		else if (DHT_type == _DHT22)
		{
			conH |= ((uint16_t)raw_data[0]<<8) | raw_data[1];
			conV |= ((uint16_t)raw_data[2]<<8) | raw_data[3];
			if( (conH >1020) || ( conH < -20) ) DHT_status = Fail_H;
			if(  (conV > 800) || ( conV < -400) ) DHT_status = Fail_T;
		}
		
  		
 	}
	if(DHT_status == OK)
	{
		for(int k = 0; k < 4; k++) _data[k] = raw_data[k];
	}
 	if(DHT_status != OK)
 	{
 		for(int k = 0; k < 4; k++) _data[k] = '\0';
 	}
}


void get_DHT_data(uint8_t humi[2], uint8_t temp[2], uint8_t _pin, uint8_t _type)
{
	uint8_t data[4];
	
	receive_raw_data(data, _pin, _type);
	
if (DHT_type == _DHT11)
{
	humi[0] = data[0] +8;
	if(humi[0]>99) humi[0] = 99;
	humi[1] = data[1];
	temp[0] = data[2];
	temp[1] = data[3];
}

else if (DHT_type == _DHT22)
{
	uint16_t _humi =0, _temp =0;
	_humi |= ((uint16_t)data[0]<<8) | data[1];
	_temp |= ((uint16_t)data[2]<<8) | data[3];
	
	humi[0] = _humi / 10;
	humi[1] = _humi % 10;
	
	temp[0] = _temp / 10;
	temp[1] = _temp % 10;
}
                        
}

float avg_hum(uint8_t _pin, uint8_t _type)
{
	uint8_t hum[7][2], tem[7][2];
	uint8_t Hmax, Hmin, Imax, Imin;
	uint16_t sum = 0;
	for(int i = 0; i<7; i++)
	{
		get_DHT_data(hum[i], tem[i], _pin, _type);
		if(i == 0)
		{
			Hmax = hum[i][0];
			Hmin = hum[i][0];
			Imax = i;
			Imin = i;
		}
		else
		{
			Imax = hum[i][0] > Hmax? i : Imax;
			Imin = hum[i][0] < Hmin? i : Imin;
		}
		sum += hum[i][0];
	}
	return (float)(sum - hum[Imax][0] - hum[Imin][0]) / 5;
}