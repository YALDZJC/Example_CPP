#include "DjiMotor.hpp"
#include <cstring> // 添加头文件

namespace Can
{
namespace Dji
{
DjiMotorBase::DjiMotorBase(int16_t address, uint8_t MotorSize, DjiMotorFeedback *MotorAddress, uint8_t *idxs)
{
    this->_Motor_ID_IDX_BIND_(idxs, MotorSize);

    this->feedback_ = MotorAddress;
    this->init_address = address;
    for (uint8_t i = 0; i < MotorSize; i++)
    {
        this->address = address + idxs[i];
    }

    this->MotorSize = MotorSize;
}

void DjiMotorBase::_Motor_ID_IDX_BIND_(uint8_t *ids, uint8_t size)
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

int DjiMotorBase::GET_Motor_ID_ADDRESS_BIND_(int address)
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

// 设置发送数据
void DjiMotorBase::setMSD(send_data *msd, int16_t data, int id)
{
    msd->Data[(id - 1) * 2] = data >> 8;
    msd->Data[(id - 1) * 2 + 1] = data << 8 >> 8;
}

void DjiMotorBase::Parse(const CAN_RxHeaderTypeDef RxHeader, const uint8_t *pData)
{
    if (!(CAN_ID(RxHeader) >= this->init_address && CAN_ID(RxHeader) <= this->init_address + 10) ||
        this->MotorSize == 0)
        return;

    int idx = GET_Motor_ID_ADDRESS_BIND_(CAN_ID(RxHeader));

    if (idx == -1)
        return; // 如果超越数组大小，或者不存在id

    uint64_t data_part_;
    std::memcpy(&data_part_, pData, sizeof(data_part_));

    auto &part1 alignas(uint64_t) = *reinterpret_cast<DjiMotorFeedback *>(&data_part_);

    feedback_[idx].angle = __builtin_bswap16(part1.angle);
    feedback_[idx].velocity = __builtin_bswap16(part1.velocity);
    feedback_[idx].current = __builtin_bswap16(part1.current);
}
} // namespace Dji
} // namespace Can