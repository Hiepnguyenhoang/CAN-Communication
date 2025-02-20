#include "UDS.h"

#define NRC 0x7F
#define INVALID_LENGTH 0x13
#define INVALID_DID 0x31
#define WRONG_KEY 0x35
#define ACCESS_DENIED 0x33

uint8_t Seed[4] = {0x01, 0x08, 0x82, 0x21};
uint8_t Key[4] = {0x00, 0x00, 0x00, 0x00};

uint8_t flag_SeedProvided = 0;
uint8_t flag_SecurityUnlocked = 0;

uint16_t newStdId = 0x0000;

uint8_t getSID(uint8_t data[])
{
    return data[1];
}

uint16_t getDID(uint8_t data[])
{
    uint16_t tmp = 0x0000;
    tmp = data[2];
    tmp <<= 8;
    tmp += data[3];
    return tmp;
}

void CAN1_Send()
{
    USART3_SendString((uint8_t *)"Tester Request: ");
    PrintCANLog(CAN1_pHeader.StdId, CAN1_DATA_TX);
    if (HAL_CAN_AddTxMessage(&hcan1, &CAN1_pHeader, CAN1_DATA_TX,
                             &CAN1_pTxMailbox) != HAL_OK)
    {
        Error_Handler();
    }
}

void CAN2_Send()
{
    PrintCANLog(CAN2_pHeader.StdId, CAN2_DATA_TX);
    if (HAL_CAN_AddTxMessage(&hcan2, &CAN2_pHeader, CAN2_DATA_TX,
                             &CAN2_pTxMailbox) != HAL_OK)
    {
        Error_Handler();
    }
}

void FormatCANFrame(uint8_t data[])
{
    for (uint8_t loopIndx = data[0] + 1; loopIndx < 8; loopIndx++)
    {
        if (data[loopIndx] == 0x00)
        {
            data[loopIndx] = 0x55;
        }
    }
}

void calculate_key_from_seed(uint8_t *input, uint8_t *output)
{
    output[0] = input[0] ^ input[1];
    output[1] = input[1] + input[2];
    output[2] = input[2] ^ input[3];
    output[3] = input[3] + input[0];
}

bool cmp_key(uint8_t *key1, uint8_t *key2, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        if (key1[i] != key2[i])
        {
            return 0;
        }
    }
    return 1;
}

bool checkFormat(uint8_t data[])
{
    // data[1] is the Service ID
    // data[0] is the length of the data
    switch (data[1])
    {
    case 0x22:
        if (data[0] == 0x03)
            return true;
        return false;
        break;
    case 0x27:
        if (data[0] == 0x02 || data[0] == 0x06)
            return true;
        return false;
        break;
    case 0x2E:
        if (data[0] == 0x05)
            return true;
        return false;
    default:
        USART3_SendString((uint8_t *)"Invalid Service ID\n");
        return false;
        break;
    }
}

bool checkDID(uint8_t data[])
{
    if (data[2] == 0x01 && data[3] == 0x23)
        return true;
    return false;
}

void UART_ReadString(uint8_t *buf, uint8_t *data, uint8_t len)
{
    if (buf[0] != 0)
    {
        data[0] = len;
        for (uint8_t i = 1; i <= len; i++)
        {
            data[i] = buf[i - 1];
        }
        for (uint8_t i = len + 1; i < 8; i++)
        {
            data[i] = 0x00;
        }
        FormatCANFrame(data);
        return;
    }
    USART3_SendString((uint8_t *)"Invalid Input\n");
}

void SID22_Practice(uint8_t *data_tx, uint8_t *data_rx)
{
    if (checkFormat(data_rx)) // check length returned true
    {
        if (checkDID(data_rx)) // check DID returned true, CAN2 Receive Correct message
        {
            // Case 1 in excel, $22 normal flow
            data_tx[0] = 0x05;
            data_tx[1] = data_rx[1] + 0x40;
            data_tx[2] = data_rx[2];
            data_tx[3] = data_rx[3];
            data_tx[4] = 0x00;
            data_tx[5] = 0x78;
            data_tx[6] = 0x00;
            data_tx[7] = 0x00;
            FormatCANFrame(data_tx);
            // Expected Res: 05 62 01 23 00 78 55 55
        }
        // Case 3 in excel, Invalid DID
        else
        {
            data_tx[0] = 0x03;
            data_tx[1] = NRC;
            data_tx[2] = 0x22;
            data_tx[3] = INVALID_DID;
            for (int i = 4; i < 8; i++)
            {
                data_tx[i] = 0x00;
            }
            FormatCANFrame(data_tx);
            // Expected Res: 03 7F 22 31 55 55 55 55
        }
    }
    // Case 2 in excel, Invalid Length
    else
    {
        data_tx[0] = 0x03;
        data_tx[1] = NRC;
        data_tx[2] = 0x22;
        data_tx[3] = INVALID_LENGTH;
        for (int i = 4; i < 8; i++)
        {
            data_tx[i] = 0x00;
        }
        FormatCANFrame(data_tx);
        // Expected Res: 03 7F 22 13 55 55 55 55
    }

    HAL_CAN_AddTxMessage(&hcan2, &CAN2_pHeader, data_tx, &CAN2_pTxMailbox);
}

void SID27_Practice(uint8_t *data_tx, uint8_t *data_rx)
{
    HAL_Delay(100);
    switch (data_rx[2])
    {
    case 0x01: // Request Seed
        if (data_rx[0] == 0x02)
        {
            data_tx[0] = 0x06;
            data_tx[1] = data_rx[1] + 0x40;
            data_tx[2] = 0x01;
            for (int i = 3; i < 7; i++)
            {
                data_tx[i] = Seed[i - 3];
            }
            data_tx[7] = 0x00;
            FormatCANFrame(data_tx);
            flag_SeedProvided = 1;
            // Expected Res: 06 67 01 55 55 55 55 55
        }
        else
        {
            data_tx[0] = 0x03;
            data_tx[1] = NRC;
            data_tx[2] = 0x27;
            data_tx[3] = INVALID_LENGTH;
            for (int i = 4; i < 8; i++)
            {
                data_tx[i] = 0x00;
            }
            FormatCANFrame(data_tx);
            // Expected Res: 03 7F 27 13 55 55 55 55
        }
        break;
    case 0x02: // Send Key
        if (data_rx[0] == 0x06)
        {
            calculate_key_from_seed(Seed, Key);
            if (cmp_key(Key, &data_rx[3], 4)) // Key match and ECU is not unlocked
            {
                if (flag_SeedProvided == 1 && flag_SecurityUnlocked == 0)
                {
                    HAL_TIM_Base_Start_IT(&htim2);
                    flag_SecurityUnlocked = 1;
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

                    USART3_SendString((uint8_t *)"Session Unlocked");

                    data_tx[0] = 0x02;
                    data_tx[1] = data_rx[1] + 0x40;
                    data_tx[2] = 0x02;
                    for (int i = 3; i < 8; ++i)
                        data_tx[i] = 0x00;
                    FormatCANFrame(data_tx);

                    flag_SeedProvided = 0;
                    
                }
                else // ECU is already unlocked
                    return;
            }

            else // Key not match
            {
                data_tx[0] = 0x03;
                data_tx[1] = NRC;
                data_tx[2] = 0x27;
                data_tx[3] = WRONG_KEY;
                for (int i = 4; i < 8; i++)
                {
                    data_tx[i] = 0x00;
                }
                FormatCANFrame(data_tx);
            }
        }
        if (data_rx[0] != 0x06)
        {
            data_tx[0] = 0x03;
            data_tx[1] = NRC;
            data_tx[2] = 0x27;
            data_tx[3] = INVALID_LENGTH;
            for (int i = 4; i < 8; i++)
            {
                data_tx[i] = 0x00;
            }
            FormatCANFrame(data_tx);
        }
        break;

    default:
        if (data_rx[0] != 0x02 || data_rx[0] != 0x06) 
        {
            data_tx[0] = 0x03;
            data_tx[1] = NRC;
            data_tx[2] = 0x27;
            data_tx[3] = INVALID_LENGTH;
            for (int i = 4; i < 8; i++)
            {
                data_tx[i] = 0x00;
            }
            FormatCANFrame(data_tx);
        }
        
        break;
    }
    HAL_CAN_AddTxMessage(&hcan2, &CAN2_pHeader, data_tx, &CAN2_pTxMailbox);
    HAL_Delay(100);
}

void SID2E_Practice(uint8_t *data_tx, uint8_t *data_rx)
{
    HAL_Delay(100);
    if (checkFormat(data_rx))
    {
        if (checkDID(data_rx))
        {
            if (flag_SecurityUnlocked == 1) // Normal Flow
            {
                data_tx[0] = 0x03;
                data_tx[1] = data_rx[1] + 0x40;
                data_tx[2] = data_rx[2];
                data_tx[3] = data_rx[3];
                data_tx[4] = 0x00;
                data_tx[5] = 0x00;
                data_tx[6] = 0x00;
                data_tx[7] = 0x00;
                FormatCANFrame(data_tx);
                // Expected Res: 03 6E 01 23 55 55 55 55
            }
            else // Access Denied because ECU is locked
            {
                data_tx[0] = 0x03;
                data_tx[1] = NRC;
                data_tx[2] = 0x2E;
                data_tx[3] = ACCESS_DENIED;
                for (int i = 4; i < 8; i++)
                {
                    data_tx[i] = 0x00;
                }
                FormatCANFrame(data_tx);
                // Expected Res: 03 7F 2E 33 55 55 55 55
            }
        }
        else
        {
            data_tx[0] = 0x03;
            data_tx[1] = NRC;
            data_tx[2] = 0x2E;
            data_tx[3] = INVALID_DID;
            for (int i = 4; i < 8; i++)
            {
                data_tx[i] = 0x00;
            }
            FormatCANFrame(data_tx);
            // Expected Res: 03 7F 2E 31 55 55 55 55
        }
    }
    else
    {
        newStdId = ((data_rx[4] << 8) | data_rx[5]) & 0x7FF;
        data_tx[0] = 0x03;
        data_tx[1] = NRC;
        data_tx[2] = 0x2E;
        data_tx[3] = INVALID_LENGTH;
        for (int i = 4; i < 8; i++)
        {
            data_tx[i] = 0x00;
        }
        FormatCANFrame(data_tx);
    }
    HAL_CAN_AddTxMessage(&hcan2, &CAN2_pHeader, data_tx, &CAN2_pTxMailbox);
    HAL_Delay(1000);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2)
    {
        if (flag_SecurityUnlocked == 0)
            return;
        else
        {
            HAL_TIM_Base_Stop_IT(&htim2);
            flag_SecurityUnlocked = 0;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
            USART3_SendString((uint8_t *)"Session Locked");
        }
    }
}