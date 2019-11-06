/*
 * HX711.c
 *
 * Created: 2019-08-08 오후 2:43:13
 *  Author: bth8188
 */ 
#include "HX711_2.h"

uint32_t offset2;
double load_scale2 = 459.7578;

void power_off2()
{
	HX711_PORT2 |= (1 << HX711_SCLK_PIN2);
	_delay_us(60);
}

void power_on2()
{
	HX711_PORT2 &= ~(1 << HX711_SCLK_PIN2);
}

void HX711_init2()
{
	//DDR setting
	HX711_DDR2 &= ~(1 << HX711_DO_PIN2);
	HX711_DDR2 |= (0x01 << HX711_SCLK_PIN2);
}

void rising_edge2()
{
	HX711_PORT2 &= ~(1 << HX711_SCLK_PIN2);
	HX711_PORT2 |= (1 << HX711_SCLK_PIN2);
}

void falling_edge2()
{
	HX711_PORT2 |= (1 << HX711_SCLK_PIN2);
	HX711_PORT2 &= ~(1 << HX711_SCLK_PIN2);
}

uint8_t Load_Cell_Ready2()
{
	power_on2();
	if(HX711_PIN2 & (1 << HX711_DO_PIN2)) return 0;
	else return 1;
}


uint32_t read_load_cell2()
{
	uint32_t temp = 0;
	uint8_t filler = 0x00, at;
	while(!Load_Cell_Ready2());
	for(int j =23; j > -1; j--)
	{
		rising_edge2();
		at = (HX711_PIN2 & (1 << HX711_DO_PIN2)) == (1 << HX711_DO_PIN2) ? 0x01 : 0x00;
		temp |= ((uint32_t)at << j);
	}
	rising_edge2();
	falling_edge2();
	if(temp & 0x800000) filler = 0xff;
	temp |= ((uint32_t)filler << 24);

	return temp;
}

uint32_t avg_value2(int num)
{
	uint32_t sum =0;
	for(int i =0; i <num; i++)
	{
		sum += (read_load_cell2() / num);
	}
	return sum;
}

void set_offset2(int _num)
{
	offset2 = avg_value2(_num);
}

uint32_t get_value2(int num)
{
	uint32_t temp = avg_value2(num) - offset2;
	if(temp>0xffff0000) temp =0;
	return temp;
}

void calc_value2(int _num, uint32_t weight[2])
{
	weight[0] = (double)get_value2(_num) / load_scale2;
	weight[1] = (uint32_t)(((double)get_value2(_num) / load_scale2) *100) %100;
}
