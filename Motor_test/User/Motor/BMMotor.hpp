#pragma once
// 基础DJI电机实现
#include "MotorBase.hpp"
#include "can.h"
#include <memory>

namespace BSP::Motor::BM
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
    double encoder_to_deg; // 编码器值转角度系数
    double encoder_to_rpm;
    double rpm_to_radps;                    // RPM转角速度系数
    double current_to_torque_coefficient;   // 电流转扭矩系数
    double feedback_to_current_coefficient; // 反馈电流转电流系数
    double deg_to_real;                     // 角度转实际角度系数

    static constexpr double deg_to_rad = 0.017453292519611;
    static constexpr double rad_to_deg = 1 / 0.017453292519611;

    // 构造函数带参数计算
    /**
     * @brief Construct a new Parameters object
     *
     * @param rr 减速比
     * @param tc 力矩常数
     * @param fmc 反馈最大电流值
     * @param mc 实际最大电流
     * @param er 编码器分辨率
     */
    Parameters(double rr, double tc, double fmc, double mc, double er)
        : reduction_ratio(rr), torque_constant(tc), feedback_current_max(fmc), current_max(mc), encoder_resolution(er)
    {
        constexpr double PI = 3.14159265358979323846;
        encoder_to_deg = 360.0 / encoder_resolution;
        rpm_to_radps = 1 / reduction_ratio / 60 * 2 * PI;
        encoder_to_rpm = 0.1;
        current_to_torque_coefficient = reduction_ratio * torque_constant / feedback_current_max * current_max / 1.414;
        feedback_to_current_coefficient = current_max / feedback_current_max;
        deg_to_real = 1 / reduction_ratio;
    }
};

/**
 * @brief 本末电机的基类
 *
 * @tparam N 电机总数
 */
template <uint8_t N> class BMMotorBase : public MotorBase<N>
{
  protected:
    /**
     * @brief Construct a new Dji Motor Base object
     *
     * @param can_id can的初始接收id ，本末从0x50, 如果id为1就是0x51，那么初始ID变为0x50，后面接着就是电机ID
     * @param params 初始化转换国际单位的参数
     */
    BMMotorBase(uint16_t Init_id, const uint8_t (&recv_idxs)[N], uint32_t send_idxs,
                Parameters params // 直接接收参数对象
                )
        : init_address(Init_id), params_(params)
    {
        // 初始化 recv_idxs_ 和 send_idxs_
        for (uint8_t i = 0; i < N; ++i)
        {
            recv_idxs_[i] = recv_idxs[i];
        }
        send_idxs_ = send_idxs;
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
        const uint16_t received_id = CAN::BSP::CAN_ID(RxHeader);

        for (uint8_t i = 0; i < N; ++i)
        {
            if (received_id == init_address + recv_idxs_[i])
            {
                std::memcpy(&feedback_[i], pData, sizeof(BMMotorfeedback));

                feedback_[i].angle = __builtin_bswap16(feedback_[i].angle);
                feedback_[i].velocity = __builtin_bswap16(feedback_[i].velocity);
                feedback_[i].current = __builtin_bswap16(feedback_[i].current);
                feedback_[i].voltage = __builtin_bswap16(feedback_[i].voltage);

                Configure(i);

                this->runTime_[i].dirTime.UpLastTime();
            }
        }
    }

    /**
     * @brief 设置发送数据
     *
     * @param msd   发送数据的结构体
     * @param torque  数据发送的扭矩
     * @param id    CAN id
     */
    void setCAN(float torque, int id)
    {
        // 将扭矩转化为发送值 扭矩 Nm = 给定电流值（A）/1.414 * 转矩常数（1.2Nm/A）
        float cur = (torque * 1.414f) / params_.torque_constant;

        auto send_data = static_cast<uint16_t>(cur * 100);

        msd.Data[(id - 1) * 2] = send_data >> 8;
        msd.Data[(id - 1) * 2 + 1] = send_data << 8 >> 8;
    }

    /**
     * @brief               发送Can数据
     *
     * @param han           Can句柄
     * @param pTxMailbox    邮箱
     */
    void sendCAN(CAN_HandleTypeDef *han, uint32_t pTxMailbox)
    {
        CAN::BSP::Can_Send(han, send_idxs_, msd.Data, pTxMailbox);
    }

    /**
     * @brief 使能BM关节电机,本末电机的使能ID为0x38
     *
     * @param hcan 电机的can句柄
     */
    void On(CAN_HandleTypeDef *hcan)
    {
        *(uint64_t *)(&msd.Data[0]) = 0x0202020202020202;

        CAN::BSP::Can_Send(hcan, 0x38, msd.Data, CAN_TX_MAILBOX2);
    }

    /**
     * @brief 使能BM关节电机,本末电机的使能ID为0x38
     *
     * @param hcan 电机的can句柄
     */
    void OFF(CAN_HandleTypeDef *hcan)
    {
        *(uint64_t *)(&msd.Data[0]) = 0x0101010101010101;

        CAN::BSP::Can_Send(hcan, 0x38, msd.Data, CAN_TX_MAILBOX2);
    }

//    /**
//     * @brief 使能BM关节电机,本末电机的使能ID为0x38
//     *
//     * @param hcan 电机的can句柄
//     */
//    void OFF(CAN_HandleTypeDef *hcan)
//    {
//        *(uint64_t *)(&msd.Data[0]) = 0x0001000000000000;

//        CAN::BSP::Can_Send(hcan, 0x38, msd.Data, CAN_TX_MAILBOX2);
//    }

  protected:
    struct alignas(uint64_t) BMMotorfeedback
    {
        int16_t velocity;
        int16_t current;
        int16_t angle;
        uint16_t voltage;
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

    // // 定义参数生成方法的虚函数
    // virtual Parameters GetParameters() = 0; // 纯虚函数要求子类必须实现

  private:
    /**
     * @brief 将反馈数据转换为国际单位
     *
     * @param i 存结构体的id号
     */
    void Configure(size_t i)
    {
        const auto &params = params_;

        this->unit_data_[i].angle_Deg = feedback_[i].angle * params.encoder_to_deg;

        this->unit_data_[i].angle_Rad = this->unit_data_[i].angle_Deg * params.deg_to_rad;

        this->unit_data_[i].velocity_Rad = feedback_[i].velocity * params.rpm_to_radps * params.encoder_to_rpm;

        this->unit_data_[i].velocity_Rpm = feedback_[i].velocity * params.encoder_to_rpm;

        this->unit_data_[i].current_A = feedback_[i].current * params.feedback_to_current_coefficient;

        this->unit_data_[i].torque_Nm = feedback_[i].current * params.current_to_torque_coefficient;

        double lastData = this->unit_data_[i].last_angle;
        double Data = this->unit_data_[i].angle_Deg;

        if (Data - lastData < -180) // 正转
            this->unit_data_[i].add_angle += (360 - lastData + Data) * params.deg_to_real;
        else if (Data - lastData > 180) // 反转
            this->unit_data_[i].add_angle += -(360 - Data + lastData) * params.deg_to_real;
        else
            this->unit_data_[i].add_angle += (Data - lastData) * params.deg_to_real;

        this->unit_data_[i].last_angle = Data;
        // 角度计算逻辑...
    }

    const int16_t init_address;   // 初始地址
    BMMotorfeedback feedback_[N]; // 反馈数据
    uint8_t recv_idxs_[N];        // ID索引
    uint32_t send_idxs_;
    CAN::BSP::send_data msd;

  public:
    Parameters params_; // 转国际单位参数列表

    uint8_t ISDir()
    {
        bool is_dir = false;
        for (uint8_t i = 0; i < N; i++)
        {
            is_dir |= this->runTime_[i].Dir_Flag = this->runTime_[i].dirTime.ISDir(100);
            this->runTime_[i].Dir_Flag = is_dir;
        }

        return is_dir;
    }
};

/**
 * @brief 配置6020电机的参数
 *
 * @tparam N 电机数量
 */
template <uint8_t N> class PB1010B : public BMMotorBase<N>
{

  public:
    // 子类构造时传递参数
    /**
     * @brief dji电机构造函数
     *
     * @param Init_id 初始ID
     * @param recv_idxs_ 电机ID列表
     */
    PB1010B(uint16_t Init_id, const uint8_t (&recv_idxs)[N], uint32_t send_idxs)
        : BMMotorBase<N>(Init_id, recv_idxs, send_idxs,
                         // 直接构造参数对象
                         Parameters(1.0, 1.2, 7500, 75, 32768))
    {
    }
};

/**
 * @brief 电机实例
 * 模板内的参数为电机的总数量，这里为假设有两个电机
 * 构造函数的第一个参数为初始ID，第二个参数为电机ID列表,第三个参数是发送的ID
 *
 */
inline PB1010B<1> MotorP1010B(0x50, {2}, 0x32);

} // namespace BSP::Motor::BM
