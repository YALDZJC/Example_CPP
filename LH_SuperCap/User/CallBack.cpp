#include "CallBack.hpp"

#include "../User/SuperCap/SuperCap.hpp"
#include "BSP_Can.hpp"
#include "can.h"

float vel;
float add_angle;

double dt;
float kp, kd;
float pid_out;
uint8_t is_on;
float rad;

BSP::SuperCap::LH_Cap cap;

void Init()
{
    CAN::BSP::Can_Init();
}

void in_while()
{

    HAL_Delay(1);
}

// can_filo0中断接收
CAN_RxHeaderTypeDef RxHeader; // can接收数据
uint8_t RxHeaderData[8] = {0};

uint16_t send_power;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxHeaderData);

    cap.Parse(RxHeader, RxHeaderData);

    cap.msd.Data[0] = send_power >> 8;
    cap.msd.Data[1] = send_power;

    cap.sendCAN(&hcan1, 0);
}