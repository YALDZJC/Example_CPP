// dwtimer.hpp
#pragma once

#include "main.h"
#include "stdint.h"
#include <cstdint>
#include <limits>

class DWTimer
{
  public:
    DWTimer(uint32_t CPU_mHz) : CPU_Freq_mHz(CPU_mHz)
    {
        Init();
    }

    float GetDeltaT();
    void DWT_CNT_Update(void);
    double DWT_GetDeltaT64();
    void DWT_SysTimeUpdate(void);
    float DWT_GetTimeline_s(void);

    float DWT_GetTimeline_ms(void);
    uint64_t DWT_GetTimeline_us(void);
    void DWT_Delay(float Delay);

  private:
    void Init();

    typedef struct
    {
        uint32_t s;
        uint16_t ms;
        uint16_t us;
    } DWT_Time;

    uint32_t CPU_Freq_mHz;
    uint32_t cnt_last;

    DWT_Time SysTime;
    uint32_t CPU_FREQ_Hz, CPU_FREQ_Hz_ms, CPU_FREQ_Hz_us;
    uint32_t CYCCNT_RountCount;
    uint32_t CYCCNT_LAST;
    uint64_t CYCCNT64;
};

inline DWTimer DWT_Timer(168);
