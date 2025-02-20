#pragma once
#ifndef UDS_H
#define UDS_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

#define BtnU HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1)

extern UART_HandleTypeDef huart3;
extern uint8_t uart3_receive;

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern CAN_TxHeaderTypeDef CAN1_pHeader;
extern CAN_RxHeaderTypeDef CAN1_pHeaderRx;
extern CAN_FilterTypeDef CAN1_sFilterConfig;
extern CAN_TxHeaderTypeDef CAN2_pHeader;
extern CAN_RxHeaderTypeDef CAN2_pHeaderRx;
extern CAN_FilterTypeDef CAN2_sFilterConfig;
extern uint32_t CAN1_pTxMailbox;
extern uint32_t CAN2_pTxMailbox;

extern TIM_HandleTypeDef htim2;

extern uint8_t CAN1_DATA_TX[8];
extern uint8_t CAN1_DATA_RX[8];
extern uint8_t CAN2_DATA_TX[8];
extern uint8_t CAN2_DATA_RX[8];

extern uint8_t MessageCounter;

extern void USART3_SendString(uint8_t * ch);
extern void PrintCANLog(uint16_t CANID, uint8_t *CAN_Frame);

extern uint16_t NumBytesReq;
extern uint8_t REQ_BUFFER[4096];
extern uint8_t  REQ_1BYTE_DATA;

extern volatile uint8_t flag_CAN1_Received;
extern volatile uint8_t flag_CAN2_Received;


/*BEGIN SUB FUNCTIONS*/
extern uint8_t getSID(uint8_t data[]);
extern uint16_t getDID(uint8_t data[]);

extern void CAN1_Send();
extern void CAN2_Send();

//Seed and key process
extern void calculate_key_from_seed(uint8_t *input, uint8_t *output);
extern bool cmp_key(uint8_t *key1, uint8_t *key2, uint8_t len);

//check functions
extern bool checkFormat(uint8_t data[]);
extern bool checkDID(uint8_t data[]);

/*END SUB FUNCTIONS*/

extern void UART_ReadString(uint8_t *buf, uint8_t *data, uint8_t len);

extern void SID22_Practice(uint8_t *data_tx, uint8_t *data_rx);
extern void SID27_Practice(uint8_t *data_tx, uint8_t *data_rx);
extern void SID2E_Practice(uint8_t *data_tx, uint8_t *data_rx);

//Other functions

// extern void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

extern void Error_Handler(void);


#endif // UDS_H