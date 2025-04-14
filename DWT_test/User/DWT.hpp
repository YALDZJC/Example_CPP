#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <limits>

namespace BSP
{

class DWTimer
{
  public:
    // 单例模式获取实例
    static DWTimer &GetInstance(uint32_t CPU_mHz = 168)
    {
        static DWTimer instance(CPU_mHz);
        return instance;
    }

    // 删除拷贝构造和赋值操作
    DWTimer(const DWTimer &) = delete;
    DWTimer &operator=(const DWTimer &) = delete;

    // 时间差计算
    float GetDeltaT(uint32_t *cnt_last);
    double GetDeltaT64(uint32_t *cnt_last);

    // 获取时间轴
    float GetTimeline_s();
    float GetTimeline_ms();
    uint64_t GetTimeline_us();

    // 延时函数
    void Delay(float seconds);

  private:
    // 构造函数私有化
    explicit DWTimer(uint32_t CPU_mHz);

    // 初始化DWT外设
    void Init();

    // 更新系统时间
    void UpdateSysTime();

    // 更新CYCCNT计数器
    void UpdateCYCCNT();

    // 时间结构体
    struct DWT_Time
    {
        uint32_t seconds;
        uint16_t milliseconds;
        uint16_t microseconds;
    };

    // 成员变量
    const uint32_t CPU_Freq_mHz; // CPU频率（MHz）
    uint32_t CPU_FREQ_Hz;        // CPU频率（Hz）
    uint32_t CPU_FREQ_Hz_ms;     // 每毫秒周期数
    uint32_t CPU_FREQ_Hz_us;     // 每微秒周期数

    std::atomic<uint32_t> CYCCNT_LAST;       // 上一次CYCCNT值
    std::atomic<uint32_t> CYCCNT_RountCount; // 溢出计数

    uint64_t CYCCNT64; // 64位计数值
    DWT_Time SysTime;  // 系统时间
};

} // namespace BSP::DWT