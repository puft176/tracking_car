#include "stm32f10x.h"                  // Device header
#include "LED.h"

uint8_t Serial_TxPacket[6];
uint8_t Serial_RxPacket[6];
uint8_t Serial_RxData,Serial_RxFlag,Serial_RxPacFlag;

uint8_t DL22_TxPacket[6];
uint8_t DL22_RxPacket[6];
uint8_t DL22_RxData,DL22_RxFlag,DL22_RxPacFlag;

void Serial_Init(void){
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//开启USART1的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//开启USART3的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//TX USART1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//RX USART1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//TX USART3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//RX USART3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;				//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//不使用硬件流控制ctrl+alt+space联想
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;			//若还需要接收则可|上RX模式
	USART_InitStructure.USART_Parity = USART_Parity_No;		//无校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 9600;				//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//不使用硬件流控制ctrl+alt+space联想
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;			//若还需要接收则可|上RX模式
	USART_InitStructure.USART_Parity = USART_Parity_No;		//无校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3,&USART_InitStructure);
	
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);	//读DR后自动清零·，无需手动清零
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);	//读DR后自动清零·，无需手动清零
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1,ENABLE);
	USART_Cmd(USART3,ENABLE);
	
}

void Serial_SendByte(uint8_t byte){
	USART_SendData(USART1,byte);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}

void DL22_SendByte(uint8_t byte){
	USART_SendData(USART3,byte);
	while(USART_GetFlagStatus(USART3,USART_FLAG_TXE) == RESET);
}

uint8_t Serial_GetRxFlag(void){
	if(Serial_RxFlag == 1){
		Serial_RxFlag = 0;
		return 1;
	}
	return 0;
}

uint8_t DL22_GetRxFlag(void){
	if(DL22_RxFlag == 1){
		DL22_RxFlag = 0;
		return 1;
	}
	return 0;
}

uint8_t Serial_GetRxPacFlag(void){
	if(Serial_RxPacFlag == 1){
		Serial_RxPacFlag = 0;
		return 1;
	}
	return 0;
}

uint8_t DL22_GetRxPacFlag(void){
	if(DL22_RxPacFlag == 1){
		DL22_RxPacFlag = 0;
		return 1;
	}
	return 0;
}
	
uint8_t Serial_GetRxData(void){
	return Serial_RxData;
}

uint8_t DL22_GetRxData(void){
	return DL22_RxData;
}
	

void USART1_IRQHandler(void){
	static uint8_t RxState = 0,pRxPacket = 0;
	if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET){
		Serial_RxData = USART_ReceiveData(USART1);
		Serial_RxFlag = 1;
		if(RxState == 0){
			if(Serial_RxData == 0xFF){
				RxState = 1;
				pRxPacket = 0;
			}
		}
		else if(RxState == 1){
			Serial_RxPacket[pRxPacket] = Serial_RxData;
			pRxPacket++;
			if(pRxPacket >= 6){
				RxState = 2;
			}
		}
		else if(RxState == 2){
			if(Serial_RxData == 0xFE){
				RxState = 0;
				Serial_RxPacFlag = 1;
			}
		}
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}


void USART3_IRQHandler(void){
	static uint8_t RxState = 0,pRxPacket = 0;
	if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET){
		DL22_RxData = USART_ReceiveData(USART3);
		DL22_RxFlag = 1;
		if(RxState == 0){
			if(DL22_RxData == 0xFF){
				RxState = 1;
				pRxPacket = 0;
			}
		}
		else if(RxState == 1){
			DL22_RxPacket[pRxPacket] = DL22_RxData;
			pRxPacket++;
			if(pRxPacket >= 6){
				RxState = 2;
			}
		}
		else if(RxState == 2){
			if(DL22_RxData == 0xFE){
				RxState = 0;
				DL22_RxPacFlag = 1;
			}
		}
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	}
}




