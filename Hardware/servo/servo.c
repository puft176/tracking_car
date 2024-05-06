#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init(void){
	PWM_Init();
}

void Servo_SetAngle(float Angle){
	PWM_SetCompare4_TIM1((Angle+135) / 270* 2000 + 500);
}


