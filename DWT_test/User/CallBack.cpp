#include "CallBack.hpp"
#include "DWT.hpp"
#include "dwt_test.h"
#include "tim.h"

double ins_dt;
double while_dt;

uint32_t last_count = 0;
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

    HAL_Delay(1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 1 */
    if (htim->Instance == TIM7)
    {
        // 获取当前系统时间
        auto &timer = BSP::DWTimer::GetInstance();
        ins_dt = timer.GetDeltaT(&last_count);
    }
    /* USER CODE END Callback 1 */
}
