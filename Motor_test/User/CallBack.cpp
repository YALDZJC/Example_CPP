#include "CallBack.hpp"
#include "../User/Motor/BMMotor.hpp"
#include "../User/Motor/DjiMotor.hpp"
#include "../User/Motor/DmMotor.hpp"
#include "BSP_Can.hpp"
#include "can.h"

float vel;
float add_angle;

double dt;
float kp, kd;
float pid_out;
uint8_t is_on;
float rad;

void Init()
{
    CAN::BSP::Can_Init();
}

void in_while()
{
    if (is_on == 1)
    {
        BSP::Motor::BM::MotorP1010B.On(&hcan1);
        HAL_Delay(10);

        is_on = false;
    }
    if (is_on == 2)
    {
        BSP::Motor::BM::MotorP1010B.OFF(&hcan1);
        HAL_Delay(10);

        is_on = false;
    }

    pid_out = kp * (add_angle - BSP::Motor::BM::MotorP1010B.getVelocityRads(1));
    rad += BSP::Motor::BM::MotorP1010B.getVelocityRads(1) * 0.002;
    BSP::Motor::BM::MotorP1010B.setCAN(vel, 1);
    BSP::Motor::BM::MotorP1010B.setCAN(pid_out, 2);
    BSP::Motor::BM::MotorP1010B.setCAN(vel, 3);
    BSP::Motor::BM::MotorP1010B.setCAN(vel, 4);

    BSP::Motor::BM::MotorP1010B.sendCAN(&hcan1, 0);
    HAL_Delay(1);
}

// can_filo0中断接收
CAN_RxHeaderTypeDef RxHeader; // can接收数据
uint8_t RxHeaderData[8] = {0};

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxHeaderData);

    BSP::Motor::Dji::Motor3508.Parse(RxHeader, RxHeaderData);

    // CAN::Motor::Dji::Motor2006.setCAN(vel, 2);
    // CAN::Motor::Dji::Motor2006.sendCAN(hcan,0x200, 0);

    // add_angle += CAN::Motor::Dji::Motor2006.getVelocityRads(2) * 0.001;

    BSP::Motor::DM::Motor4310.Parse(RxHeader, RxHeaderData);
    BSP::Motor::BM::MotorP1010B.Parse(RxHeader, RxHeaderData);
}