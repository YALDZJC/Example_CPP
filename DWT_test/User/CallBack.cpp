#include "CallBack.hpp"
#include "DWT.hpp"
#include "dwt_test.h"
#include "tim.h"

double time;
double ins_dt;
uint32_t last_count = 0;


uint32_t INS_DWT_Count = 0;

void Init()
{
//	DWTimer::Instance().Initialize(168);  // 168 MHz主频
	HAL_TIM_Base_Start_IT(&htim7);
	DWT_Init(168);
}

void in_while()
{

//		// 测量代码段执行时间
//		uint32_t marker = DWTimer::Instance().CycleCount();
//		// ...执行代码...
//		float elapsed = DWTimer::Instance().DeltaSec(marker);
			ins_dt = DWT_GetDeltaT(&INS_DWT_Count);


			HAL_Delay(1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 1 */
  if (htim->Instance == TIM7) {
		// 获取当前系统时间
//    auto time = DWTimer::Instance().DeltaSec(last_count);

  }
  /* USER CODE END Callback 1 */
}

