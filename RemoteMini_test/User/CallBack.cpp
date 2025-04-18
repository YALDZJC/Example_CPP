#include "CallBack.hpp"
#include "Dbus.hpp"

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    Remote::dr16.Parse(huart, Size);
}
