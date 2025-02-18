
#pragma once
// 基础DJI电机实现
#include "../User/MotorBase.hpp"
#include "Bsp_Can.hpp"

#include "can.h"
#include <cstring> // 添加头文件

namespace CAN
{
namespace Motor
{
namespace Dji
{

template <uint8_t N> class DjiMotorBase
{
  public:
    void Parse(const CAN_RxHeaderTypeDef RxHeader, const uint8_t *pData)
    {
        const uint16_t received_id = BSP::CAN_ID(RxHeader);

        for (uint8_t i = 0; i < N; ++i)
        {
            if (received_id == init_address + idxs[i])
            {
                memcpy(&feedback_[i], pData, sizeof(DjiMotorfeedback));

                feedback_[i].angle = __builtin_bswap16(feedback_[i].angle);
                feedback_[i].velocity = __builtin_bswap16(feedback_[i].velocity);
                feedback_[i].current = __builtin_bswap16(feedback_[i].current);

                break;
            }
        }
        Configure();
    }
    // 设置id
    void setMSD(BSP::send_data *msd, int16_t data, int id)
    {
        msd->Data[(id - 1) * 2] = data >> 8;
        msd->Data[(id - 1) * 2 + 1] = data << 8 >> 8;
    }

  protected:
    struct UnitData
    {
        double angle_Deg;
        double last_angle;
        double add_angle;
        double velocity_Rad;
        double current_A;
        double torque_Nm;
        double temperature_C;
    };

    struct alignas(uint64_t) DjiMotorfeedback
    {
        int16_t angle;
        int16_t velocity;
        int16_t current;
        uint8_t temperature;
        uint8_t unused;
    };

    DjiMotorfeedback feedback_[N];
    UnitData unit_data_[N];

    const int16_t init_address;
    uint8_t idxs[N];

    DjiMotorBase(uint16_t can_id, const uint8_t (&ids)[N]) : init_address(can_id)
    {
        for (uint8_t i = 0; i < N; ++i)
        {
            idxs[i] = ids[i];
        }
    }

    // 参数结构体定义
    struct Parameters
    {
        double reduction_ratio;      // 减速比
        double torque_constant;      // 力矩常数 (Nm/A)
        double feedback_current_max; // 反馈最大电流 (A)
        double current_max;          // 最大电流 (A)
        double encoder_resolution;   // 编码器分辨率

        // 自动计算的参数
        double encoder_to_deg;                  // 编码器值转角度系数
        double rpm_to_radps;                    // RPM转角速度系数
        double current_to_torque_coefficient;   // 电流转扭矩系数
        double feedback_to_current_coefficient; // 反馈电流转电流系数

        // 构造函数带参数计算
        Parameters(double rr, double tc, double fmc, double mc, double er)
            : reduction_ratio(rr), torque_constant(tc), feedback_current_max(fmc), current_max(mc),
              encoder_resolution(er)
        {
            constexpr double PI = 3.14159265358979323846;
            encoder_to_deg = 360.0 / encoder_resolution;
            rpm_to_radps = 1 / reduction_ratio / 60 * 2 * PI;
            current_to_torque_coefficient = reduction_ratio * torque_constant / feedback_current_max * current_max;
            feedback_to_current_coefficient = current_max / feedback_current_max;
        }
    };

    // 参数计算函数（可被子类复用）
    Parameters CreateParams(double rr, double tc, double fmc, double mc, double er) const
    {
        return Parameters(rr, tc, fmc, mc, er);
    }

  public:
    virtual void Configure() = 0;

    float getAngle(uint8_t id) 
    {
        return this->unit_data_[id].angle_Deg;
    }
    float getLastAngle(uint8_t id) 
    {
        return 0;
    }
    float getAddAngle(uint8_t id) 
    {
        return 0;
    }

    float getCurrent(uint8_t id) 
    {
        return this->unit_data_[id].current_A;
    }
    float getTorque(uint8_t id) 
    {
        return this->unit_data_[id].torque_Nm;
    }
    float getTemperature(uint8_t id) 
    {
        return this->unit_data_[id].temperature_C;
    }
};

template <uint8_t N> class GM2006 : public DjiMotorBase<N>
{
  private:
    typename DjiMotorBase<N>::Parameters params_; // 显式指定作用域

    enum class Type : uint8_t
    {
        GM6020,
        GM6020_VOLTAGE,
        M3508,
        M2006
    };

    void Configure() override
    {
        params_ = this->CreateParams(3.0,               // 不同减速比
                                     0.18 * 1.0 / 36.0, // 不同力矩常数
                                     16384,             // 不同最大电流
                                     10, 8192);

        for (uint8_t i = 0; i < N; ++i)
        {
            this->unit_data_[i].angle_Deg = this->feedback_[i].angle * params_.encoder_to_deg;
            this->unit_data_[i].velocity_Rad = this->feedback_[i].velocity * params_.rpm_to_radps;
            this->unit_data_[i].current_A = this->feedback_[i].current * params_.feedback_to_current_coefficient;
            this->unit_data_[i].torque_Nm = this->unit_data_[i].current_A * params_.current_to_torque_coefficient;

            this->unit_data_[i].temperature_C = this->feedback_[i].temperature;

            double lastData = this->unit_data_[i].last_angle;
            double Data = this->unit_data_[i].angle_Deg;

            if (Data - lastData < -180) // 正转
                this->unit_data_[i].add_angle += (360 - lastData + Data);
            else if (Data - lastData > 180) // 反转
                this->unit_data_[i].add_angle += -(360 - Data + lastData);
            else
                this->unit_data_[i].add_angle += (Data - lastData);

            this->unit_data_[i].last_angle = Data;
        }
    }

  public:
    GM2006(uint16_t can_id, const uint8_t (&ids)[N])
        : DjiMotorBase<N>(can_id, ids), params_(this->CreateParams(3.0, 0.18 * 1.0 / 36.0, 16384, 10, 8192))
    {
    }


};

// 创建包含2个电机的实例（ID 1和2）
CAN::Motor::Dji::GM2006<2> Motor2006(0x200, {1, 2});

} // namespace Dji
} // namespace Motor
} // namespace CAN