#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "menu.h"
#include "Serial.h"
#include "LED.h"
#include "servo.h"
#include "motor.h"
#include "timer.h"
#include "Encoder.h"

extern uint8_t state;
int main(void)
{
	/*模块初始化*/
	OLED_Init();		
	Menu_Init();
	Serial_Init();
	Servo_Init();
	Motor_Init();
//	Timer_Init();	定时器和左轮1编码器B都使用了TIM3
	Encoder_Init();
	Servo_SetAngle(0);
	while (1)
	{	
//		OLED_ShowString(1,1,"speed1:");
//		OLED_ShowString(2,1,"speed2:");
//		OLED_ShowSignedNum(1,8,Encoder4_Get(),5);
//		OLED_ShowSignedNum(2,8,Encoder3_Get(),5);
		menu1();
		if(state == 1) Menu2_Xunji();
		if(state == 2) Menu2_Zhuizong();
		if(state == 3) Menu2_thrchoose();
		if(state == 4) Menu2_remote();
	}
}
