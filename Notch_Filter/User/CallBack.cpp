#include "CallBack.hpp"
#include "BSP_Can.hpp"
#include "DjiMotor.hpp"
#include "DmMotor.hpp"
#include "PID.hpp"
#include "Tools.hpp"
#include "can.h"
uint8_t send_str2[64];

float vel;
float add_angle;

double dt;

Kpid_t Kpid_vel;
PID pid_vel;

NotchFilter filter_vel;

float sin_tick,sin_hz,sin_value;

void Init()
{
    CAN::BSP::Can_Init();
    filter_vel.initNotchFilter(0.01, 0.001, 10000);
}

void in_while()
{
    // CAN::Motor::DM::Motor4310.On(&hcan1, 1);
    // CAN::Motor::DM::Motor4310.ctrl_Motor(&hcan1, 1, 0, 0, 0, 0, vel);
    // HAL_Delay(10);
}

// can_filo0中断接收
CAN_RxHeaderTypeDef RxHeader; // can接收数据
uint8_t RxHeaderData[8] = {0};

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxHeaderData);

    CAN::Motor::Dji::Motor3508.Parse(RxHeader, RxHeaderData);
		sin_tick+=0.001;
		sin_value = sinf(2*3.14159926*sin_hz*sin_tick)+sinf(2*3.14159926*sin_tick*0.01);
        filter_vel.processNotchFilter(CAN::Motor::Dji::Motor3508.getVelocityRpm(1));
        pid_vel.GetPidPos(Kpid_vel, vel, CAN::Motor::Dji::Motor3508.getVelocityRpm(1), 16384.0);

        Tools.vofaSend(CAN::Motor::Dji::Motor3508.getVelocityRpm(1), vel, filter_vel.y1, sin_value, 0, 0);

        CAN::Motor::Dji::Motor3508.setCAN(pid_vel.pid.cout, 4);
        CAN::Motor::Dji::Motor3508.sendCAN(&hcan1, 0x200, 0);
}