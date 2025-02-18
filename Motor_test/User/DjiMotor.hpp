
#pragma once
// 基础DJI电机实现
#include "../User/MotorBase.hpp"

#include "can.h"
#include <cstring> // 添加头文件

#define _Motor2006_SIZE 2

// 获取下标
#define GET_Motor_ID_IDX_BIND_(_motor_, id) (_motor_.id_idx_bind.idxs[id])
#define _Motor_ID_IDX_BIND_SIZE_ 5

namespace Can
{
inline uint32_t CAN_ID(const CAN_RxHeaderTypeDef &rx_header)
{
    return rx_header.StdId;
}

// 电机发送数据
typedef struct
{
    uint8_t Data[8];
} send_data;

namespace Dji
{

template <uint8_t N> class DjiMotorBase
{
  public:
    struct alignas(uint64_t) DjiMotorFeedback
    {
        int16_t angle;
        int16_t velocity;
        int16_t current;
        uint8_t temperature;
        uint8_t unused;
    };

    void Parse(const CAN_RxHeaderTypeDef RxHeader, const uint8_t *pData)
    {
        const uint16_t received_id = CAN_ID(RxHeader);

        for (uint8_t i = 0; i < N; ++i)
        {
            if (received_id == init_address + idxs[i])
            {
                memcpy(&feedback_[i], pData, sizeof(DjiMotorFeedback));
                feedback_[i].angle = __builtin_bswap16(feedback_[i].angle);
                feedback_[i].velocity = __builtin_bswap16(feedback_[i].velocity);
                feedback_[i].current = __builtin_bswap16(feedback_[i].current);

                break;
            }
        }
    }
    // 设置id
    void setMSD(send_data *msd, int16_t data, int id)
    {
        msd->Data[(id - 1) * 2] = data >> 8;
        msd->Data[(id - 1) * 2 + 1] = data << 8 >> 8;
    }

  protected:
    const int16_t init_address;
    uint8_t idxs[N]; // 电机最大个数

    DjiMotorBase(uint16_t can_id, const uint8_t (&ids)[N]) : init_address(can_id)
    {
        for (uint8_t i = 0; i < N; ++i)
        {
            idxs[i] = ids[i];
        }
    }

    DjiMotorFeedback feedback_[N];

  public:
    void _Motor_ID_IDX_BIND_(uint8_t *ids, uint8_t size)
    {
        uint8_t idxs[_Motor_ID_IDX_BIND_SIZE_];

        for (uint8_t i = 0; i < _Motor_ID_IDX_BIND_SIZE_; i++) // 标记
        {
            this->idxs[i] = 0xff;
        }
        for (uint8_t i = 0; i < size; i++) // 绑定
        {
            this->idxs[ids[i]] = i;
        }
    }
    // 获取对应的下标
    int GET_Motor_ID_ADDRESS_BIND_(int address)
    {
        int idx = address - (this->init_address);
        if (idx < 0)
            return -1;
        if (idx >= _Motor_ID_IDX_BIND_SIZE_)
            return -1;
        if (this->idxs[idx] == 0xff)
            return -1;

        return this->idxs[idx];
    }

    virtual float getAngle(float n) = 0;
    virtual float getLastAngle(float n) = 0;
    virtual float getAddAngle(float n) = 0;

    virtual float getCurrent(float n) = 0;
    virtual float getTorque(float n) = 0;
    virtual float getTemperature(float n) = 0;
};

template <uint8_t N> class GM2006 : public DjiMotorBase<N>
{
  public:
    GM2006(uint16_t can_id, const uint8_t (&ids)[N]) : DjiMotorBase<N>(can_id, ids)
    {
    }
    float getAngle(float n) override
    {
        return 0;
    }
    float getLastAngle(float n) override
    {
        return 0;
    }
    float getAddAngle(float n) override
    {
        return 0;
    }

    float getCurrent(float n) override
    {
        return 0;
    }
    float getTorque(float n) override
    {
        return 0;
    }
    float getTemperature(float n) override
    {
        return 0;
    }
};

// 创建包含2个电机的实例（ID 1和2）
Can::Dji::GM2006<2> Motor2006(0x200, {1, 2});

} // namespace Dji

} // namespace Can