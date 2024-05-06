#include "stm32f10x.h"                  // Device header
#include "oled.h"
#include "key.h"
#include "menu.h"
#include "Serial.h"
#include "servo.h"
#include "servo_track.h"
#include "motor.h"
#include "linetrack.h"
#include "balltrack.h"
#include "timer.h"
#include "encoder.h"
#include "delay.h"

uint8_t num = 1;

extern uint8_t function1[][16],function2[][16],function3[][16],function4[][16],function5[][16],function6[][16],function_no[][16];	//字模数据
//滚屏显示所需变量
int8_t i_show;	
//主菜单函数所需变量
uint8_t cursor = 2,last_cursor = 0,actual_cursor = 1,function_sum = 6,state = 0;
uint8_t flag_no = 0,flag_show = 0;
//二级菜单函数所需变量
uint8_t cursor2,last_cursor2,actual_cursor2,function_sum2,state2;
//舵机云台所需变量
float y_angle;


void Menu_Init(void)
{
//	char *function[6] = {"陀螺仪","串口数据收发","ADC采集","PWM输出","双机串口通信"};
	Key_Init();
	OLED_ShowString(1,6,"-MENU-");
	OLED_ShowString(cursor,1,"->");
	OLED_ShowYuju(2,3,function1,6);
	OLED_ShowYuju(3,3,function2,6);
	OLED_ShowYuju(4,3,function3,6);
}

/**
  *@brief 主菜单函数，在死循环中检测按键键码
  *@param 无
  *@retval 进入的二级菜单对应的state值
  */

uint8_t menu1(void)
{
	uint8_t keynum;
	OLED_Clear();
	OLED_ShowString(1,6,"-MENU-");
	OLED_ShowString(cursor,1,"->");
	OLED_ShowYuju(2,3,function1,6);
	OLED_ShowYuju(3,3,function2,6);
	OLED_ShowYuju(4,3,function3,6);
	while(1){
		keynum = Key_GetNum();
		/*下翻键反馈*/
		if(keynum == 1){				//下翻键按下
			last_cursor = cursor;		//光标移动前记录		
			actual_cursor ++;			//2,3,4,5,6实际光标增加
			cursor++;					//光标自增				
			if(actual_cursor > function_sum){		//功能触底仍下翻
				actual_cursor = 1;					//实际光标复位
				cursor = 2;							//光标复位
				Menu_Roll();						//滚动显示屏
			}
			else if(cursor == 5){					//功能未触底，光标触底后仍下翻
				cursor = 4;
				Menu_Roll();						//滚动显示屏
			}
			Menu_CursorMove();						//移动光标			
		}
		/*确定键反馈*/
		if(keynum == 2){
			state = actual_cursor;
			OLED_Clear();
			return state;
		}
	}
}

uint8_t Menu2_Xunji(void)
{
	uint8_t keynum,speed_set = 0;
	int8_t* speed;
	cursor2 = 1;
	last_cursor2 = 1;
	state2 = 1;
	
	Serial_SendByte(46);				//发送进入循迹模式命令
	OLED_Clear();
	Servo_SetAngle(90);
	OLED_ShowString(1,1,"->Kp:");
	OLED_ShowNum(1,6,linetrack_Kp*100,3);
	OLED_ShowString(2,3,"Speed:");
	OLED_ShowNum(2,9,speed_set,2);
	OLED_ShowString(3,3,"Kd:");
	OLED_ShowNum(3,6,linetrack_Kd*100,3);
	
	while(1){
		keynum = Key_GetNum();
		if(Serial_GetRxFlag() == 1){
			if(Serial_GetRxData()){
				speed = PD_linetrack(Serial_GetRxData());
				Motor_SetSpeed1(speed[0] + speed_set);
				Motor_SetSpeed2(speed[1] + speed_set);
			}
			else{
				Motor_SetSpeed(0);
			}			
		}
		/*下翻键反馈*/
		if(keynum == 1){
			cursor2 ++;
			state2 ++;
			if(cursor2 >= 4){		//复位
				cursor2 = 1;
				state2 = 1;
			}
			OLED_ShowString(last_cursor2,1,"  ");
			OLED_ShowString(cursor2,1,"->");
			last_cursor2 = cursor2;
		}
			
		/*返回键反馈*/
		if(keynum == 2){
			Serial_SendByte(47);				//发送退出循迹模式命令
			state = 0;		//返回主菜单
			return state;
		}
		/*按键3反馈*/
		if(keynum == 3){
			if(state2 == 1){linetrack_Kp -= 0.01;OLED_ShowNum(1,6,linetrack_Kp*100,3);}
			if(state2 == 2){speed_set -= 5;OLED_ShowNum(2,9,speed_set,2);}
			if(state2 == 3){linetrack_Kd -= 0.01;OLED_ShowNum(3,6,linetrack_Kd*100,3);}
		}
		/*按键4反馈*/
		if(keynum == 4){
			if(state2 == 1){linetrack_Kp += 0.01;OLED_ShowNum(1,6,linetrack_Kp*100,3);}
			if(state2 == 2){speed_set += 5;OLED_ShowNum(2,9,speed_set,2);}
			if(state2 == 3){linetrack_Kd += 0.01;OLED_ShowNum(3,6,linetrack_Kd*100,3);}
		}
	}
}

uint8_t Menu2_Zhuizong(void)
{
	/*变量初始化*/
	uint8_t keynum,speed_set = 20,find_angle = 20,flag = 0;
	int8_t* speed;
	cursor2 = 1;last_cursor2 = 1;state2 = 1;
	uint16_t ballarea = 0;
	int8_t encoder1,encoder2;
	int16_t integration_angle = 0;
	/*模式初始化*/
	Serial_SendByte(39);				//发送进入小球追踪模式命令
	OLED_Clear();
	OLED_ShowString(1,1,"->Kp:");
	OLED_ShowNum(1,6,balltrack_Kp*100,3);
	OLED_ShowString(2,3,"Speed:");
	OLED_ShowNum(2,9,speed_set,2);
	OLED_ShowString(3,3,"Kd:");
	OLED_ShowNum(3,6,balltrack_Kd*100,3);
	Servo_SetAngle(20);
	while(1){
		if(Serial_GetRxPacFlag() == 1){
			/*视野内无色块*/
			if(Serial_RxPacket[0] == 0 && Serial_RxPacket[1] == 1){	
				/*旋转寻找色块*/
				Motor_SetSpeed1(18);
				Motor_SetSpeed2(-18);
				encoder1 = Encoder3_Get();
				encoder2 = Encoder4_Get();
				integration_angle += (encoder1 - encoder2);	//1~0.0668°
				OLED_ShowSignedNum(4,1,integration_angle,1);
				if(integration_angle >= 450){
					integration_angle = 0;
					Motor_SetSpeed(0);
					Delay_s(5);
				}
			}
			/*找到色块*/
			else{
				/*y轴云台追踪*/
				//中心坐标(160,120),(0xA0,0x78)
				y_angle += PD_servotrack(Serial_RxPacket[1],0x78);
				if(y_angle<= -80) y_angle = -80;
				if(y_angle>= 80) y_angle = 80;
				Servo_SetAngle(y_angle);
				/*小车x轴，距离追踪*/
				ballarea = Serial_RxPacket[2]*255 + Serial_RxPacket[3];
				if(ballarea <= 12000) speed_set = 20;		//距离大于10cm，以20速度追踪
				else speed_set = 0;							//距离小于10cm，停止
				/*x轴小车追踪*/
				speed = PD_balltrack(Serial_RxPacket[0]);
				Motor_SetSpeed1(speed[0] + speed_set);
				Motor_SetSpeed2(speed[1] + speed_set);
			}				
		}
		/*按键检测*/
		keynum = Key_GetNum();
		/*下翻键反馈*/
		if(keynum == 1){
			cursor2 ++;
			state2 ++;
			if(cursor2 >= 4){		//复位
				cursor2 = 1;
				state2 = 1;
			}
			OLED_ShowString(last_cursor2,1,"  ");
			OLED_ShowString(cursor2,1,"->");
			last_cursor2 = cursor2;
		}
		/*返回键反馈*/
		if(keynum == 2){
			state = 0;		//返回主菜单
			Serial_SendByte(40);				//发送退出小球追踪命令
			Motor_SetSpeed1(0);
			Motor_SetSpeed2(0);	
			return state;
		}
		/*按键3反馈*/
		if(keynum == 3){
			if(state2 == 1){balltrack_Kp -= 0.01;OLED_ShowNum(1,6,balltrack_Kp*100,3);}
			if(state2 == 2){speed_set -= 5;OLED_ShowNum(2,9,speed_set,2);}
			if(state2 == 3){balltrack_Kd -= 0.01;OLED_ShowNum(3,6,balltrack_Kd*100,3);}
		}
		/*按键4反馈*/
		if(keynum == 4){
			if(state2 == 1){balltrack_Kp += 0.01;OLED_ShowNum(1,6,balltrack_Kp*100,3);}
			if(state2 == 2){speed_set += 5;OLED_ShowNum(2,9,speed_set,2);}
			if(state2 == 3){balltrack_Kd += 0.01;OLED_ShowNum(3,6,balltrack_Kd*100,3);}
		}
	}
}

uint8_t Menu2_thrchoose(void)
{
	uint8_t keynum;
	cursor2 = 2;
	function_sum2 = 2;
	actual_cursor2 = 1;
	OLED_Clear();
	OLED_ShowString(cursor2,1,"->");
	OLED_ShowString(2,3,"white");
	OLED_ShowString(3,3,"black");
	while(1){
		keynum = Key_GetNum();
		/*下翻键反馈*/
		if(keynum == 1){				//下翻键按下
			last_cursor2 = cursor2;		//光标移动前记录		
			actual_cursor2 ++;			//2,3,4,5,6实际光标增加
			cursor2++;					//光标自增				
			if(actual_cursor2 > function_sum2){		//功能触底仍下翻
				actual_cursor2 = 1;					//实际光标复位
				cursor2 = 2;							//光标复位
			}
			Menu2_CursorMove();						//移动光标			
		}
		/*确定键反馈*/
		if(keynum == 2){
			state2 = actual_cursor2;
			OLED_Clear();
			Menu3_thrset();
			return 0;
		}
	}	
}

uint8_t Menu2_remote(void)
{
	uint8_t command_y = 0,command_x = 0,keynum;
	int16_t speed1 = 0,speed2 = 0,speed = 0;
	OLED_ShowString(1,1,"Command:");
	while(1){
		keynum = Key_GetNum();
		if(DL22_GetRxPacFlag() == 1){
			command_x = DL22_RxPacket[0];
			command_y = DL22_RxPacket[1];
			
			if(command_x == 43){		//左转
				speed1 = 15;
				speed2 = -15;
			}
			else if(command_x == 44){	//右转
				speed1 = -15;
				speed2 = 15;
			}
			else{
				speed1 = 0;
				speed2 = 0;
			}
			if(command_y == 41) speed = 30;		//前进
			if(command_y == 42){					//后退时朝摇杆方向转弯
				speed = -30;
				speed1 = -speed1;
				speed2 = -speed2;
			}
			if(command_y == 45) speed = 0;
			Motor_SetSpeed1(speed + speed1);
			Motor_SetSpeed2(speed + speed2);
			OLED_ShowNum(1,9,command_x,2);
			OLED_ShowNum(1,12,command_y,2);
		}
		/*确定键反馈*/
		if(keynum == 2){
			return 0;
		}
	}
}

uint8_t Menu3_thrset(void)
{
	uint8_t keynum,lastrow = 2,lastcolumn = 3,state3 = 1;
	static int8_t Lmax = 100,Lmin = 0,Amax = 127,Amin = -128,Bmax = 127,Bmin = -128;
	int16_t LAB[7] = {0,Lmin,Lmax,Amin,Amax,Bmin,Bmax};
	OLED_ShowString(1,5,"-thrset-");
	OLED_ShowString(2,3,"->");
	OLED_ShowString(2,1,"L:");
	OLED_ShowString(3,1,"A:");
	OLED_ShowString(4,1,"B:");
	
//	OLED_ShowSignedNum(2,5,Lmin,3);
//	OLED_ShowSignedNum(3,5,Amin,3);
//	OLED_ShowSignedNum(4,5,Bmin,3);
//	OLED_ShowSignedNum(2,12,Lmax,3);
//	OLED_ShowSignedNum(3,12,Amax,3);
//	OLED_ShowSignedNum(4,12,Bmax,3);
	
	Serial_SendByte(1);				//发送进入阈值调试命令
									
	
	while(1){
		keynum = Key_GetNum();
		/*阈值数据包接收*/
		if(Serial_GetRxPacFlag() == 1){
			LAB[1] = Serial_RxPacket[0];
			LAB[2] = Serial_RxPacket[1];
			LAB[3] = Serial_RxPacket[2];
			LAB[4] = Serial_RxPacket[3];
			LAB[5] = Serial_RxPacket[4];
			LAB[6] = Serial_RxPacket[5];
			OLED_ShowSignedNum(2,5,LAB[1],3);
			OLED_ShowSignedNum(2,12,LAB[2],3);
			OLED_ShowSignedNum(3,5,LAB[3],3);
			OLED_ShowSignedNum(3,12,LAB[4],3);
			OLED_ShowSignedNum(4,5,LAB[5],3);
			OLED_ShowSignedNum(4,12,LAB[6],3);
			
		}

		/*下翻键反馈*/
		if(keynum == 1){
			OLED_ShowString(lastrow,lastcolumn,"  ");
			state3 += 1;
			if(state3 == 7) state3 = 1;
			if(state3 % 2 == 1){			//左列
				OLED_ShowString((state3+1)/2 + 1,3,"->");
				lastrow = (state3+1)/2 + 1;
				lastcolumn = 3;
			}
			else{
				OLED_ShowString(state3/2 + 1,10,"->");
				lastrow = state3/2 + 1;
				lastcolumn = 10;
			}
		}
		/*返回键反馈*/
		if(keynum == 2){
			Serial_SendByte(2);				//发送退出阈值调试命令
			state = 0;		//返回主菜单
			return state;
		}
		/*减少键反馈*/
		if(keynum == 3){
			LAB[state3] -= 5;
//			OLED_ShowSignedNum(lastrow,lastcolumn+2,LAB[state3],3);
			Serial_SendByte(2+state3*2-1+(state2-1)*12);		//发送命令调节阈值%d的第%d值，state2:1~3，state3:1~6
			
		}
		/*增加键反馈*/
		if(keynum == 4){
			LAB[state3] += 5;
//			OLED_ShowSignedNum(lastrow,lastcolumn+2,LAB[state3],3);
			Serial_SendByte(2+state3*2+(state2-1)*12);		//发送命令调节阈值%d的第%d值，state2:1~3，state3:1~6
		}
		
	}
}


void Menu_Roll(void){
	static int8_t i = 2;
	if(i==-1)		//上次滚动已经触底
		i=3;		//复位
	i--;			//i： 2，1，0 -> 2，1，0 
	OLED_ShowYuju(i,3,function1,6);
	OLED_ShowYuju(i+1,3,function2,6);
	OLED_ShowYuju(i+2,3,function3,6);
	OLED_ShowYuju(i+3,3,function4,6);
	OLED_ShowYuju(i+4,3,function5,6);
	OLED_ShowYuju(i+5,3,function6,6);
	
	i_show = i;
}

void Menu_Show(int8_t i){
	OLED_ShowString(1,6,"-MENU-");
	OLED_ShowYuju(i,3,function1,6);
	OLED_ShowYuju(i+1,3,function2,6);
	OLED_ShowYuju(i+2,3,function3,6);
	OLED_ShowYuju(i+3,3,function4,6);
	OLED_ShowYuju(i+4,3,function5,6);
	OLED_ShowYuju(i+5,3,function6,6);
	OLED_ShowString(cursor,1,"->");
}

void Menu_Show_no(void){
	OLED_Clear();
	OLED_ShowYuju(2,5,function_no,4);
}

void Menu_CursorMove(void)
{
	if(last_cursor != 0){
		OLED_ShowString(last_cursor,1,"  ");
		OLED_ShowString(cursor,1,"->");
		last_cursor = 0;
	}	
}

void Menu2_CursorMove(void)
{
	if(last_cursor2 != 0){
		OLED_ShowString(last_cursor2,1,"  ");
		OLED_ShowString(cursor2,1,"->");
		last_cursor = 0;
	}
}

