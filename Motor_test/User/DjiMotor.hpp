// 基础DJI电机实现
#include "../User/MotorBase.hpp"

#include "can.h"

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

struct alignas(uint64_t) DjiMotorFeedback
{
    int16_t angle;
    int16_t velocity;
    int16_t current;
    uint8_t temperature;
    uint8_t unused;
};

class DjiMotorBase
{
  public:
    DjiMotorBase(int16_t address, uint8_t MotorSize, DjiMotorFeedback *MotorAddress, uint8_t *idxs);

    void Parse(const CAN_RxHeaderTypeDef RxHeader, const uint8_t *pData);
    // 设置id
    void setMSD(send_data *msd, int16_t data, int id);

  public:
    void _Motor_ID_IDX_BIND_(uint8_t *ids, uint8_t size);
    // 获取对应的下标
    int GET_Motor_ID_ADDRESS_BIND_(int address);

    virtual float getAngle(float n) = 0;
    virtual float getLastAngle(float n) = 0;
    virtual float getAddAngle(float n) = 0;

    virtual float getCurrent(float n) = 0;
    virtual float getTorque(float n) = 0;
    virtual float getTemperature(float n) = 0;

  protected:
    DjiMotorFeedback *feedback_;
    int16_t init_address;
    int16_t address; // 地址

    uint8_t MotorSize;
    uint8_t idxs[_Motor_ID_IDX_BIND_SIZE_]; // 电机最大个数
};

class GM2006 : public DjiMotorBase
{
  public:
    GM2006(int16_t address, uint8_t MotorSize, DjiMotorFeedback *MotorAddress, uint8_t *idxs)
        : DjiMotorBase(address, MotorSize, MotorAddress, idxs)
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

inline DjiMotorFeedback _Motor2006_[_Motor2006_SIZE];
inline uint8_t _Motor2006_ID_[_Motor2006_SIZE] = {1, 2};

inline GM2006 Motor2006(0x200, _Motor2006_SIZE, _Motor2006_, _Motor2006_ID_);

} // namespace Dji

} // namespace Can