#include "stm32f10x.h"                  // Device header

float PD_servotrack(uint8_t Y_measure,uint8_t Y_calcu)
{
	int16_t erro_y,last_erro_y = 0;
	float angle_vari = 0,Kp = -0.07,Kd = 0.05; //kp- kd+ -0.07 0.05
	erro_y = Y_measure - Y_calcu;
	angle_vari = Kp*erro_y + Kd*(erro_y-last_erro_y);
	return angle_vari;
}

