#include "CallBack.hpp"
#include "BSP_Can.hpp"
#include "DjiMotor.hpp"
#include "can.h"

void Init()
{
    CAN::BSP::Can_Init();
}


// can_filo0中断接收
CAN_RxHeaderTypeDef RxHeader; // can接收数据
uint8_t RxHeaderData[8] = {0};

float vel;
float add_angle;

double dt;
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxHeaderData);

    CAN::Motor::Dji::Motor2006.Parse(RxHeader, RxHeaderData);

    CAN::Motor::Dji::Motor2006.setCAN(vel, 2);
    CAN::Motor::Dji::Motor2006.sendCAN(hcan, 0x200, 0);

    add_angle += CAN::Motor::Dji::Motor2006.getVelocityRads(2) * 0.001;

}