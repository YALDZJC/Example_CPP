#include "can.h"
#include <memory>

namespace BSP::SuperCap
{
class LH_Cap
{
  public:
    void Parse(const CAN_RxHeaderTypeDef RxHeader, const uint8_t *pData)
    {
        if (RxHeader.StdId == 233)
        {
            std::memcpy(&feedback_, pData, sizeof(feedback));

            feedback_.Chassis_Voltage = __builtin_bswap16(feedback_.Chassis_Voltage);
            feedback_.Cap_Voltage = __builtin_bswap16(feedback_.Cap_Voltage);
            feedback_.Power = __builtin_bswap16(feedback_.Power);
        }
    }

    struct alignas(uint64_t) feedback
    {
        int16_t Chassis_Voltage;
        int16_t Cap_Voltage;
        int16_t Power;

        uint8_t State;
        uint8_t Is_On;
    };

    enum class State : uint8_t
    {
        NORMAL,
        WARNING,
        ERROR
    };

    enum class Switch : uint8_t
    {
        ENABLE,
        DISABLE
    };

    feedback feedback_;

    State CapState = State::NORMAL;
    Switch CapSwitch = Switch::DISABLE;


  private:
};
} // namespace BSP::SuperCap
