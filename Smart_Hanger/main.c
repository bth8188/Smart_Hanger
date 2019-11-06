/*
 * Smart_Hanger.c
 *
 * Created: 2019-08-26 오후 5:03:19
 * Author : kccistc
 */ 
#include "main.h"

enum {Waiting, Running} Main_State;
enum {ready, aroll, shake, measure, broll} Run_State;
	
ISR(TIMER0_COMP_vect)
{
	incMilliSec();
	incTime();
}

ISR(USART1_RX_vect)
{
	BT_ISR_Receive();
}

ISR(INT6_vect)
{
	ISR_Toggle_A();
}

ISR(INT7_vect)
{
	ISR_Toggle_B();
}

int main(void)
{
	char lcd1[18], lcd2[18], Rxbuff[50], Txbuff[10];
	uint32_t checker = 0, lcd_time=0, dht_time = 0, load_time = 0, weight1[2], weight2[2], total_gram, First_weight = 0, servo_time,
	Lost_weight = 0, total;
	uint8_t dht_flag = 0, start_Flag =0, servo_Flag = 0, dry_per = 0, night_Flag, auto_Flag = 1, shake_times = 0, humi[2], temper[2], ex1[2], ex2[2], *BT_buff, *cmd_buff;
	float external_humid, cloth_humid, current_temper, First_humid =0;
	
    sei();
	
	DDRA |= (1 << 0);	// lcd 전원
	lcd_on();
	BT_Init();
	DCmotorInit();
	ADC_Init();
	buzzerInit();
	timer0init();
	servoInit();
	HX711_init();
	HX711_init2();
	HallSensor_Init();
	Button_Init(start_stop_button);
	
	Main_State = Waiting;
	Run_State = ready;
	
	sprintf(lcd1, "initialize");
	sprintf(lcd2, ".....");
	I2C_LCD_write_string_XY(0, 0, lcd1);
	I2C_LCD_write_string_XY(1, 0, lcd2);
	
	set_offset(20);
	set_offset2(20);
	BT_buff = Rxbuff;
	cmd_buff = Txbuff;
	start_sound();
    while (1) 
    {
		checker = millis();
		if(Main_State == Waiting)
		{
			Blower_Fan(0);
			lcd_off();
			DDRB &= ~(1 << 5);
			if(checker - load_time > 10000);
			{
				calc_value(5, weight1);
				calc_value2(5, weight2);
				total_gram = weight1[0] + weight2[0];
				if((total_gram > 10) && (start_Flag == 0))
				{
					Main_State = Running;
					lcd_on();
				}
				else if((total_gram <= 50) && (start_Flag ==1) )
				{
					set_offset(20);
					set_offset2(20);
					start_Flag = 0;
				}
				sprintf(BT_buff, "%ldg, start \n", total_gram);
				BT_printf_string(BT_buff);
				load_time = millis();
			}
		}
		
		else
		{
			switch(Run_State)
			{
				case ready:
					if(checker - load_time > 3000);
					{
						calc_value(5, weight1);
						calc_value2(5, weight2);
						total_gram = weight1[0] + weight2[0];
						First_weight = total_gram;
						sprintf(lcd1, " %u g Hanging ", total_gram);
						sprintf(lcd2, " Press start   ");
						get_DHT_data(humi,temper, 0, 2);
						First_humid = humi[0] + (float)humi[1]/10;
						load_time = millis();
						sprintf(BT_buff, "First_humid = %d.%d%c\n", humi[0], humi[1], '%');
						BT_printf_string(BT_buff);
						sprintf(BT_buff, "Total_gram = %d\n", total_gram);
						BT_printf_string(BT_buff);
						//실험
					}
					if(start_Flag)
					{
						Run_State = aroll;
						set_rotation(0);
						ready_sound();
					}
					break;
				
				case aroll:
					DCmotorright(80);
					sprintf(lcd1, "Start Dry     ");
					sprintf(lcd2, "rolling...    ");
					if(get_rotation() >= 500)
					{
						DCmotorstop();
						Run_State = shake;
						set_rotation(0);
						dht_flag =1;
					}
					break;
				
				case shake:
					DDRB |= (1 << 5);
					if(checker - servo_time >800) servo_Flag ^= 1;
					// 흔들기
					sprintf(lcd1, "%d.%d%c   %d.%dC   ", ex1[0], ex1[1], '%' ,ex2[0], ex2[1]);
					sprintf(lcd2, "%ld mL dried  ", Lost_weight);
					if(night_Flag == 0) 
					{
						sprintf(BT_buff,"%d \n", shake_times);
						BT_printf_string(BT_buff);
						shake_times++;
						total++;
						if(servo_Flag)
						{
							servo_run(30);
							_delay_ms(500);
						}
						else
						{
							servo_run(90);
							_delay_ms(500);
						}
					}
					if(shake_times >= 6)
					{
						shake_times = 0;
						if(Run_State != broll) Run_State = measure;
					}
					break;
				
				case measure:
					// 수치 측정
					BT_printf_string("measure");
					calc_value(5, weight1);
					calc_value2(5, weight2);
					total_gram = weight1[0] + weight2[0];
					Lost_weight = (First_weight - total_gram) > Lost_weight? (First_weight - total_gram) : Lost_weight;
					get_DHT_data(humi,temper, 0, 2);
					get_DHT_data(ex1, ex2, 1, 1);
					// 수치 발송
					sprintf(BT_buff, "Lweight %d extemp %d.%d humi %d.%d\n", Lost_weight, ex1[0], ex1[1], humi[0], humi[1]);
					BT_printf_string(BT_buff);
					sprintf(lcd1, "%d.%d%c %d.%dC", humi[0], humi[1], '%', temper[0], temper[1]);
					sprintf(lcd2, "%               ");
					sprintf(BT_buff, "%d.%d%c %d.%dC %d %d\n", humi[0], humi[1], '%', temper[0], temper[1], ex1[0], ex2[0]);
					BT_printf_string(BT_buff);
					// 복귀
					if(total > 60)
					{
						Run_State = broll;
						total = 0;
					}
					if(humi[0] < 75) Run_State = broll;
					else
					{
						if(Run_State != broll) Run_State = shake;
					}
					cloth_humid = humi[0] + (float)humi[1]/10;
					break;
				
				case broll:
					//reset
					dry_per = 0;
					First_weight = 0;
					Lost_weight = 0;
					cloth_humid = 0;
					shake_times = 0;
					
					DCmotorleft(80);
					if(get_rotation() > 500)
					{
						DCmotorstop();
						Run_State = ready;
						Main_State = Waiting;
					}
					break;
			}
			// 블루투스 명령어
			if(isBTString())
			{
				cmd_buff = getBTtring();
				if(cmd_buff[0] = 'at')
				{
					start_Flag = 1;
					Main_State = Running;
				}
				else if(cmd_buff[0] = 'bt')
				{
					total = 61;
					set_rotation(0);
				}
				else if(cmd_buff[0] = 'ct')
				{
					auto_Flag = 0;
					night_Flag =1;
				}
				else if(cmd_buff[0] = 'dt')
				{
					auto_Flag = 1;
					night_Flag = 0;
				}
			}
			//버튼 작동
			if(button(start_stop_button))
			{
				if(start_Flag)
				{
					Run_State = broll;
					set_rotation(0);
				}
				else start_Flag = 1;
			}
			//밤인지 확인
// 			if(auto_Flag)
// 			{
// 				if(read_ADC(0) >= 800) night_Flag = 0;
// 				else night_Flag =1;
// 			}
		}
		// 블로워 팬 작동
		if(start_Flag)
		{
			if(total_gram > 100) Blower_Fan(255);
			else if(total_gram > 50) Blower_Fan(200);
			else Blower_Fan(180);
		}
		if(checker-lcd_time > 500)
		{
			if(isLcd_Flag())
			{
				I2C_LCD_write_string_XY(0, 0, lcd1);
				I2C_LCD_write_string_XY(1, 0, lcd2);
				lcd_time = millis();
				external_humid = avg_hum(1, 1);
				get_DHT_data(humi, temper, 0, 2);
				cloth_humid = humi[0] + (humi[1]/10);
			}
		}
    }
}

