#include "stm32f10x.h"                  // Device header

float balltrack_Kp = 0.20,balltrack_Kd = 0.14;

int8_t* PD_balltrack(uint16_t average_x)
{
	static int8_t speed[2];		//[speed1,speed2]
	
	static int16_t error_x,lasterror_x = 0;
	average_x *= 2;					//320 ~ 160 ~ 0
	error_x = 160 - average_x;		//-160 ~ 0 ~ 160
//	speed[0] = error_x * Kp;
//	speed[1] = error_x * -Kp;
	speed[0] = error_x * balltrack_Kp + (error_x - lasterror_x)*-balltrack_Kd;
	speed[1] = error_x * -balltrack_Kp + (error_x - lasterror_x)*balltrack_Kd;
	return speed;
}
