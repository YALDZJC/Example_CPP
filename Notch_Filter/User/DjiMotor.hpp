
#pragma once
// 基础DJI电机实现
#include "Bsp_Can.hpp"
#include "MotorBase.hpp"
#include "can.h"
#include <cstdint>
#include <cstring> // 添加头文件

namespace CAN
{
namespace Motor
{
namespace Dji
{
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

    static constexpr double deg_to_rad = 0.017453292519611;
    static constexpr double rad_to_deg = 1 / 0.017453292519611;

    // 构造函数带参数计算
    Parameters(double rr, double tc, double fmc, double mc, double er)
        : reduction_ratio(rr), torque_constant(tc), feedback_current_max(fmc), current_max(mc), encoder_resolution(er)
    {
        constexpr double PI = 3.14159265358979323846;
        encoder_to_deg = 360.0 / encoder_resolution;
        rpm_to_radps = 1 / reduction_ratio / 60 * 2 * PI;
        current_to_torque_coefficient = reduction_ratio * torque_constant / feedback_current_max * current_max;
        feedback_to_current_coefficient = current_max / feedback_current_max;
    }
};

/**
 * @brief 大疆电机的基类
 *
 * @tparam N 电机总数
 */
template <uint8_t N> class DjiMotorBase : public MotorBase<N>
{
  protected:
    /**
     * @brief Construct a new Dji Motor Base object
     *
     * @param can_id can的初始id 比如3508与20066就是0x200
     * @param params 初始化转换国际单位的参数
     */
    DjiMotorBase(uint16_t Init_id, const uint8_t (&ids)[N], Parameters params) : init_address(Init_id), params_(params)
    {
        for (uint8_t i = 0; i < N; ++i)
        {
            idxs[i] = ids[i];
        }
    }

  public:
    // 解析函数
    /**
     * @brief 解析CAN数据
     *
     * @param RxHeader  接收数据的句柄
     * @param pData     接收数据的缓冲区
     */
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

                Configure(i);
                break;
            }
        }
    }

    /**
     * @brief 设置发送数据
     *
     * @param msd   发送数据的结构体
     * @param data  数据发送的数据
     * @param id    CAN id
     */
    void setCAN(int16_t data, int id)
    {
        msd.Data[(id - 1) * 2] = data >> 8;
        msd.Data[(id - 1) * 2 + 1] = data << 8 >> 8;
    }

    /**
     * @brief               发送Can数据
     *
     * @param han           Can句柄
     * @param StdId         Can发送id
     * @param pTxMailbox    邮箱
     */
    void sendCAN(CAN_HandleTypeDef *han, uint32_t StdId, uint32_t pTxMailbox)
    {
        BSP::Can_Send(han, StdId, msd.Data, pTxMailbox);
    }

  protected:
    struct alignas(uint64_t) DjiMotorfeedback
    {
        int16_t angle;
        int16_t velocity;
        int16_t current;
        uint8_t temperature;
        uint8_t unused;
    };

    /**
     * @brief Create a Params object
     *
     * @param rr 减速比
     * @param tc 力矩常数
     * @param fmc 反馈电流最大值
     * @param mc 真实电流最大值
     * @param er 编码器分辨率
     * @return Parameters
     */
    Parameters CreateParams(double rr, double tc, double fmc, double mc, double er) const
    {
        return Parameters(rr, tc, fmc, mc, er);
    }

    // 定义参数生成方法的虚函数
    virtual Parameters GetParameters() = 0; // 纯虚函数要求子类必须实现

  private:
    /**
     * @brief 将反馈数据转换为国际单位
     *
     * @param i 存结构体的id号
     */
    void Configure(size_t i)
    {
        const auto &params = GetParameters();

        this->unit_data_[i].angle_Deg = feedback_[i].angle * params.encoder_to_deg;

        this->unit_data_[i].angle_Rad = this->unit_data_[i].angle_Deg * params.deg_to_rad;

        this->unit_data_[i].velocity_Rad = feedback_[i].velocity * params.rpm_to_radps;
        this->unit_data_[i].velocity_Rpm = feedback_[i].velocity;

        this->unit_data_[i].current_A = feedback_[i].current * params.feedback_to_current_coefficient;

        this->unit_data_[i].torque_Nm = this->unit_data_[i].current_A * params.current_to_torque_coefficient;

        this->unit_data_[i].temperature_C = feedback_[i].temperature;

        double lastData = this->unit_data_[i].last_angle;
        double Data = this->unit_data_[i].angle_Deg;

        if (Data - lastData < -180) // 正转
            this->unit_data_[i].add_angle += (360 - lastData + Data);
        else if (Data - lastData > 180) // 反转
            this->unit_data_[i].add_angle += -(360 - Data + lastData);
        else
            this->unit_data_[i].add_angle += (Data - lastData);

        this->unit_data_[i].last_angle = Data;
        // 角度计算逻辑...
    }

    const int16_t init_address;    // 初始地址
    DjiMotorfeedback feedback_[N]; // 反馈数据
    uint8_t idxs[N];               // ID索引
    Parameters params_;            // 转国际单位参数列表
    BSP::send_data msd;

  public:

};

/**
 * @brief 配置2006电机的参数
 *
 * @tparam N 电机数量
 */
template <uint8_t N> class GM2006 : public DjiMotorBase<N>
{
  private:
    // 定义参数生成方法
    Parameters GetParameters() override
    {
        return DjiMotorBase<N>::CreateParams(36.0, 0.18 * 1.0 / 36.0, 16384, 10, 8192);
    }

  public:
    // 子类构造时传递参数
    /**
     * @brief dji电机构造函数
     *
     * @param Init_id 初始ID
     * @param ids 电机ID列表
     */
    GM2006(uint16_t Init_id, const uint8_t (&ids)[N]) : DjiMotorBase<N>(Init_id, ids, GetParameters())
    {
    }
};

/**
 * @brief 配置3508电机的参数
 *
 * @tparam N 电机数量
 */
template <uint8_t N> class GM3508 : public DjiMotorBase<N>
{
  private:
    // 定义参数生成方法
    Parameters GetParameters() override
    {
        return DjiMotorBase<N>::CreateParams(19.0, 0.3 * 1.0 / 19.0, 16384, 20, 8192);
    }

  public:
    // 子类构造时传递参数
    /**
     * @brief dji电机构造函数
     *
     * @param Init_id 初始ID
     * @param ids 电机ID列表
     */
    GM3508(uint16_t Init_id, const uint8_t (&ids)[N]) : DjiMotorBase<N>(Init_id, ids, GetParameters())
    {
    }
};

/**
 * @brief 配置6020电机的参数
 *
 * @tparam N 电机数量
 */
template <uint8_t N> class GM6020 : public DjiMotorBase<N>
{
  private:
    // 定义参数生成方法
    Parameters GetParameters() override
    {
        return DjiMotorBase<N>::CreateParams(1.0, 0.7 * 1.0, 16384, 3, 8192);
    }

  public:
    // 子类构造时传递参数
    /**
     * @brief dji电机构造函数
     *
     * @param Init_id 初始ID
     * @param ids 电机ID列表
     */
    GM6020(uint16_t Init_id, const uint8_t (&ids)[N]) : DjiMotorBase<N>(Init_id, ids, GetParameters())
    {
    }
};

/**
 * @brief 电机实例
 * 模板内的参数为电机的总数量，这里为假设有两个电机
 * 构造函数的第一个参数为初始ID，第二个参数为电机ID列表
 *
 */
CAN::Motor::Dji::GM2006<2> Motor2006(0x200, {1, 2});
CAN::Motor::Dji::GM3508<1> Motor3508(0x200, {4});
CAN::Motor::Dji::GM6020<2> Motor6020(0x204, {1, 2});

} // namespace Dji
} // namespace Motor
} // namespace CAN
