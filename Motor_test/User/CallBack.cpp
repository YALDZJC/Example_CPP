#include "CallBack.hpp"
#include "BSP_Can.hpp"
#include "DjiMotor.hpp"
#include "DmMotor.hpp"
#include "can.h"

float vel;
float add_angle;

double dt;
void Init()
{
    CAN::BSP::Can_Init();
}

void in_while()
{
    CAN::Motor::DM::Motor4310.On(&hcan1, 1);
    CAN::Motor::DM::Motor4310.ctrl_Motor(&hcan1, 1, 0, 0, 0, 0, vel);
    HAL_Delay(10);
}

// can_filo0中断接收
CAN_RxHeaderTypeDef RxHeader; // can接收数据
uint8_t RxHeaderData[8] = {0};


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxHeaderData);

    // CAN::Motor::Dji::Motor2006.Parse(RxHeader, RxHeaderData);

    // CAN::Motor::Dji::Motor2006.setCAN(vel, 2);
    // CAN::Motor::Dji::Motor2006.sendCAN(hcan,0x200, 0);

    // add_angle += CAN::Motor::Dji::Motor2006.getVelocityRads(2) * 0.001;

    CAN::Motor::DM::Motor4310.Parse(RxHeader, RxHeaderData);
}