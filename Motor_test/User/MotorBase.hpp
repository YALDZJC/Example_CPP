#include "can.h"

// 抽象电机接口
class IMotor
{
  public:
    virtual ~IMotor() = default;

    virtual void Parse(const CAN_RxHeaderTypeDef &rx_header, const uint8_t data[]) = 0;
    virtual void sendData(CAN_HandleTypeDef *handle) = 0;

    virtual float getTorque() const = 0;
    virtual void setTarget(float value, Dji_Data dataType) = 0;

    virtual uint16_t getCanId() const = 0;
    virtual MotorType getType() const = 0;
};

// 基础DJI电机实现
class DjiMotorBase : public IMotor
{
  protected:
    enum class Type : uint8_t
    {
        GM6020,
        GM6020_VOLTAGE,
        M3508,
        M2006
    };

    struct alignas(uint64_t) DjiMotorFeedback
    {
        int16_t angle;
        int16_t velocity;
        int16_t current;
        uint8_t temperature;
        uint8_t unused;
    };

    Type MotorType;
    FeedbackData feedback_;
    uint16_t canId_;

  public:
    DjiMotorBase(uint16_t canId, MotorConfig config) : canId_(canId), config_(config)
    {
    }

    FeedbackData getFeedback() const override
    {
        return feedback_;
    }

    uint16_t getCanId() const override
    {
        return canId_;
    }

  protected:
    virtual void decodeFrame(const uint8_t data[]) = 0;
    virtual void encodeFrame(uint8_t data[]) const = 0;

  private:
    void Parse(const CAN_RxHeaderTypeDef &rx_header, const uint8_t data[]) override
    {
    }
};