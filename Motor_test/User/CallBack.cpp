#include "CallBack.hpp"
#include "BSP_Can.hpp"
#include "DjiMotor.hpp"
#include "can.h"

// 创建0x200类型的电机
void Init()
{
    Can_Init();
}

// can_filo0中断接收
CAN_RxHeaderTypeDef RxHeader; // can接收数据
uint8_t RxHeaderData[8] = {0};
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxHeaderData);

    Can::Dji::Motor2006.Parse(RxHeader, RxHeaderData);
}