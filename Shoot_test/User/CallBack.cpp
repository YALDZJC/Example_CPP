#include "CallBack.hpp"
#include "../User/Motor/DjiMotor.hpp"
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

}

// can_filo0中断接收
CAN_RxHeaderTypeDef RxHeader; // can接收数据
uint8_t RxHeaderData[8] = {0};

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{


    /* USER CODE BEGIN Callback 1 */
    if (htim->Instance == TIM7)
    {
        // 变量声明
        auto Dail_vel = BSP::Motor::Dji::Motor2006.getVelocityRpm(1);
        auto Shoot_L_vel = BSP::Motor::Dji::Motor3508.getVelocityRpm(1);
        auto Shoot_R_vel = BSP::Motor::Dji::Motor3508.getVelocityRpm(2);

        


        BSP::Motor::Dji::Motor2006.setCAN();
        BSP::Motor::Dji::Motor3508.setCAN();
        BSP::Motor::Dji::Motor3508.setCAN();

        BSP::Motor::Dji::Motor3508.sendCAN(&hcan1, 0);
        BSP::Motor::Dji::Motor2006.sendCAN(&hcan1, 1);
    }
    /* USER CODE END Callback 1 */
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxHeaderData);
		
    BSP::Motor::Dji::Motor3508.Parse(RxHeader, RxHeaderData);
    BSP::Motor::Dji::Motor2006.Parse(RxHeader, RxHeaderData);

    
}