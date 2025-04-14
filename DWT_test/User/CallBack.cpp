#include "CallBack.hpp"
#include "DWT.hpp"
#include "tim.h"

double callbcak_dt;
double while_dt;

uint32_t callback_count = 0;
uint32_t while_count = 0;

void Init()
{
    HAL_TIM_Base_Start_IT(&htim7);
    auto &timer = BSP::DWTimer::GetInstance(168);
}

void in_while()
{
    // 测量代码段执行时间
    auto &timer = BSP::DWTimer::GetInstance();
    while_dt = timer.GetDeltaT(&while_count);

    timer.Delay(0.0001);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 1 */
    if (htim->Instance == TIM7)
    {
        // // 获取当前系统时间
        // auto &timer = BSP::DWTimer::GetInstance();
        // callbcak_dt = timer.GetDeltaT(&callback_count);
    }
    /* USER CODE END Callback 1 */
}
