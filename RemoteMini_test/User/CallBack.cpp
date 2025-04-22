#include "CallBack.hpp"
#include "Dbus.hpp"
#include "../User/MimiRemote/Mini.hpp"

BSP::Remote::Mini remoteMini;

void Init()
{
remoteMini.Init();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // Remote::dr16.Parse(huart, Size);
    remoteMini.Parse(huart, Size);
}
