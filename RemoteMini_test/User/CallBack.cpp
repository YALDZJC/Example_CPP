#include "CallBack.hpp"
#include "../User/Key/KeyBroad.hpp"
#include "../User/MimiRemote/Mini.hpp"
#include "Dbus.hpp"

void Init()
{
    // 获取实例
    auto &remote = BSP::Remote::Mini::getInstance();
    remote.Init();
}

float Lx;
BSP::Key::SimpleKey key;
uint32_t Click;
uint32_t LongPress;
uint32_t KeepClick;
uint32_t RisingEdge;
uint32_t FallingEdge;
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // Remote::dr16.Parse(huart, Size);
    auto &remote = BSP::Remote::Mini::getInstance();

    remote.Parse(huart, Size);

    Lx = remote.remoteLeft().x;

    key.update(remote.paused() == BSP::Remote::Mini::Switch::UP);

    if (key.getClick())
    {
        Click++;
    }
    if (key.getLongPress())
    {
        LongPress++;
    }
    if (key.getToggleState())
    {
        KeepClick++;
    }
}
