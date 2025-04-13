// dwtimer.hpp
#pragma once
#include <cstdint>
#include <limits>
#include "main.h"
#include "stdint.h"
class DWTimer
{
  public:
    struct Time
    {
        uint32_t seconds;
        uint16_t milliseconds;
        uint16_t microseconds;
    };

    // 删除拷贝构造函数和赋值操作符
    DWTimer(const DWTimer &) = delete;
    DWTimer &operator=(const DWTimer &) = delete;

    // 获取单例实例
    static DWTimer &Instance()
    {
        static DWTimer instance;
        return instance;
    }

    // 初始化计时器（CPU主频，单位MHz）
    void Initialize(uint32_t cpu_freq_mhz);

    // 获取时间差（秒）
    float DeltaSec(uint32_t &last_count);
    double DeltaSec64(uint32_t &last_count);

    // 获取系统时间
    Time SystemTime() const;
    float SystemTimeSec();
    float SystemTimeMillisec();
    uint64_t SystemTimeMicrosec();

    // 延时功能
    void DelaySec(float seconds);
    void DelayMillisec(uint32_t milliseconds);

    // 当前周期计数
    uint32_t CycleCount() const;

  private:
    DWTimer() = default; // 私有构造函数

    void UpdateCycleCounters();
    void UpdateSystemTime();

    uint32_t cpu_freq_hz_ = 0;
    uint32_t cpu_freq_khz_ = 0;
    uint32_t cpu_freq_mhz_ = 0;

    mutable uint32_t cycle_rounds_ = 0;
    mutable uint32_t last_cycle_count_ = 0;
    mutable Time system_time_ = {0, 0, 0};
};

