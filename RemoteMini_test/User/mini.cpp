#include "mini.hpp"
#include "memory"
void init()
{
    // Remote::BSP::Mini.Init();
}

namespace BSP::Remote
{
void Mini::Init()
{
    HAL_UARTEx_ReceiveToIdle_DMA(&ClickerHuart, pData, sizeof(pData));
}

/**
 * @brief 利用memcpy保存数据
 *
 * @param pData 缓冲区
 */
void Mini::SaveData(const uint8_t *pData)
{
    auto copyData = [&pData](auto &dest) {
        std::memcpy(&dest, pData, sizeof(dest));
        pData += sizeof(dest);
    };

    uint16_t part0;
    copyData(part0);
    data_part0_ = part0;

    uint64_t part1;
    copyData(part1);
    data_part1_ = part1;
    // 跳过两个字节，因为遥控器内存不连续
    pData += 2;

    uint64_t part2;
    copyData(part2);
    data_part2_ = part2;

    // 跳过两个字节，因为遥控器内存不连续
    pData += 4;

    uint64_t part3;
    copyData(part3);
    data_part3_ = part3;
}

/**
 * @brief 更新遥控器状态
 * 使用位域结构体将data_part强转为结构体，再赋值到对应的结构体成员
 * 使用C++风格reinterpret_cast强转指针
 */
void Mini::UpdateStatus()
{
    auto &part1 alignas(uint64_t) = *reinterpret_cast<MiniDataPart1 *>(&data_part1_);
    auto channel_to_double = [](uint16_t value) { return (static_cast<int32_t>(value) - 1024) / 660.0; };

    joystick_right_.y = -channel_to_double(static_cast<uint16_t>(part1.joystick_channel0));
    joystick_right_.x = channel_to_double(static_cast<uint16_t>(part1.joystick_channel1));
    joystick_left_.y = -channel_to_double(static_cast<uint16_t>(part1.joystick_channel2));
    joystick_left_.x = channel_to_double(static_cast<uint16_t>(part1.joystick_channel3));

    // 拨杆值
    sw_.x = channel_to_double(static_cast<uint16_t>(part1.sw));
    // 挡位开关
    gear_ = static_cast<Gear>(part1.gear);
    // 暂停按键
    paused_ = static_cast<Switch>(part1.paused);
    // 左右自定义按键
    fn_left_ = static_cast<Switch>(part1.fn_left);
    fn_right_ = static_cast<Switch>(part1.fn_right);

		trigger_ = static_cast<Switch>(part1.trigger);
		
    // 鼠标速度
    auto &part2 alignas(uint64_t) = *reinterpret_cast<MiniDataPart2 *>(&data_part2_);
    mouse_vel_.x = part2.mouse_velocity_x / 32768.0;
    mouse_vel_.y = part2.mouse_velocity_y / 32768.0;
    // 鼠标按键
    mouse_.left = part2.mouse_left;
    mouse_.right = part2.mouse_right;

    // 键盘按键
    auto &part3 alignas(uint64_t) = *reinterpret_cast<MiniDataPart3 *>(&data_part3_);
    keyboard_ = part3.keyboard;
}

/**
 * @brief 数据解析
 *
 * @param huart 对应串口号
 * @param Size 数据的大小
 */
void Mini::Parse(UART_HandleTypeDef *huart, int Size)
{
    // 本机遥控器
    if (huart == &ClickerHuart && Size == sizeof(pData))
    {
        SaveData(pData);
        UpdateStatus();
    }
    HAL_UARTEx_ReceiveToIdle_DMA(&ClickerHuart, pData, sizeof(pData));
}

/**
 * @brief 清除ORE错误
 *
 * @param huart 对应串口handle
 * @param pData 缓冲区
 * @param Size 数据大小
 */
void Mini::ClearORE(UART_HandleTypeDef *huart, uint8_t *pData, int Size)
{
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) != RESET)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
        HAL_UARTEx_ReceiveToIdle_DMA(huart, pData, Size);
    }
}

} // namespace BSP::Remote
