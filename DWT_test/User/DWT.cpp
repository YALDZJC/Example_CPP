// dwtimer.cpp
#include "DWT.hpp"

void DWTimer::Initialize(uint32_t cpu_freq_mhz)
{
    // 启用DWT跟踪
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // 重置周期计数器
    DWT->CYCCNT = 0;

    // 启用周期计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    cpu_freq_hz_ = cpu_freq_mhz * 1'000'000;
    cpu_freq_khz_ = cpu_freq_hz_ / 1'000;
    cpu_freq_mhz_ = cpu_freq_khz_ / 1'000;
}

uint32_t DWTimer::CycleCount() const
{
    return DWT->CYCCNT;
}

void DWTimer::UpdateCycleCounters()
{
    const uint32_t current = CycleCount();
    if (current < last_cycle_count_)
    {
        ++cycle_rounds_;
    }
    last_cycle_count_ = current;
}

void DWTimer::UpdateSystemTime()
{
    UpdateCycleCounters();
    const uint64_t total_cycles =
        static_cast<uint64_t>(cycle_rounds_) * std::numeric_limits<uint32_t>::max() + CycleCount();

    system_time_.seconds = total_cycles / cpu_freq_hz_;
    const uint64_t remainder = total_cycles % cpu_freq_hz_;

    system_time_.milliseconds = remainder / cpu_freq_khz_;
    system_time_.microseconds = (remainder % cpu_freq_khz_) / cpu_freq_mhz_;
}

DWTimer::Time DWTimer::SystemTime() const
{
    const_cast<DWTimer *>(this)->UpdateSystemTime();
    return system_time_;
}

float DWTimer::SystemTimeSec()
{
    UpdateSystemTime();
    return static_cast<float>(system_time_.seconds) + system_time_.milliseconds * 1e-3f +
           system_time_.microseconds * 1e-6f;
}

float DWTimer::SystemTimeMillisec()
{
    UpdateSystemTime();
    return system_time_.seconds * 1e3f + system_time_.milliseconds + system_time_.microseconds * 1e-3f;
}

uint64_t DWTimer::SystemTimeMicrosec()
{
    UpdateSystemTime();
    return static_cast<uint64_t>(system_time_.seconds) * 1'000'000ULL + system_time_.milliseconds * 1'000ULL +
           system_time_.microseconds;
}

float DWTimer::DeltaSec(uint32_t &last_count)
{
    const uint32_t current = CycleCount();
    const float delta = static_cast<float>(current - last_count) / cpu_freq_hz_;
    last_count = current;
    UpdateCycleCounters();
    return delta;
}

double DWTimer::DeltaSec64(uint32_t &last_count)
{
    const uint32_t current = CycleCount();
    const double delta = static_cast<double>(current - last_count) / cpu_freq_hz_;
    last_count = current;
    UpdateCycleCounters();
    return delta;
}

void DWTimer::DelaySec(float seconds)
{
    const uint32_t start = CycleCount();
    const uint32_t wait_cycles = static_cast<uint32_t>(seconds * cpu_freq_hz_);
    while ((CycleCount() - start) < wait_cycles)
        ;
}

void DWTimer::DelayMillisec(uint32_t milliseconds)
{
    const uint32_t start = CycleCount();
    const uint32_t wait_cycles = milliseconds * cpu_freq_khz_;
    while ((CycleCount() - start) < wait_cycles)
        ;
}
