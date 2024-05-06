#ifndef __SERIAL_H__
#define __SERIAL_H__

extern uint8_t Serial_TxPacket[];
extern uint8_t Serial_RxPacket[];
extern uint8_t DL22_TxPacket[];
extern uint8_t DL22_RxPacket[];

void Serial_Init(void);

void Serial_SendByte(uint8_t byte);
uint8_t Serial_GetRxData(void);
uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxPacFlag(void);

void DL22_SendByte(uint8_t byte);
uint8_t DL22_GetRxData(void);
uint8_t DL22_GetRxFlag(void);
uint8_t DL22_GetRxPacFlag(void);


#endif
