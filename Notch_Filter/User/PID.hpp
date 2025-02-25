#pragma once
#include "math.h"
#include "stdint.h"
class TD
{
  public:
    float u;
    float x1, x2, max_x2;
    float r, h, r2_1;
    TD(float r = 1.0f, float max_x2 = 0, float h = 0.001f) : r(r), h(h), max_x2(max_x2)
    {
    }

    void Calc(float u);

  private:
};

// 调参kp,ki,kd结构体
struct Kpid_t
{
    double kp, ki, kd;
    Kpid_t(double kp = 0, double ki = 0, double kd = 0) : kp(kp), ki(ki), kd(kd)
    {
    }
};

typedef struct
{
    // 期望，实际
    double cin, cout, feedback;
    // p,i,d计算
    double p, i, d;
    // Delta,p,i,d计算
    double Dp, Di, Dd;
    // 误差
    double last_e, last_last_e, now_e;
    // td跟踪微分器，跟踪误差
    TD td_e;
    // 限幅
    double MixI;
    // 积分隔离
    float Break_I;
} Pid_t;

class PID
{
  private:
    /* data */
  public:
    Pid_t pid;
    PID(double Ierror = 0, double MixI = 0)
    {
        this->pid.Break_I = Ierror;
        this->pid.MixI = MixI;

        this->pid.td_e.r = 100;
    }
    // 位置式pid获取
    double GetPidPos(Kpid_t kpid, double cin, double feedback, double max);
    // 清除pid
    void clearPID();
    // 清除增量
    void PidRstDelta();

    inline float GetErr()
    {
        return this->pid.now_e;
    }

    inline float GetCout()
    {
        return this->pid.cout;
    }
    inline float GetCin()
    {
        return this->pid.cin;
    }
};

class FeedForward
{
  protected:
    // 输出
    float k;
    // 输出限幅
    float max_cout;

  public:
    float cout;
    FeedForward(float max_cout, float k) : max_cout(max_cout), k(k), cout(0)
    {
    }
    double GetCout()
    {
        return cout;
    }
};

class FeedTar : public FeedForward
{
  public:
    // 上一次目标
    double last_target;
    double new_target;
    // 目标误差
    double target_e;

    double last_cout;
    double cout_e;
    FeedTar(float max_cout, float k) : FeedForward(max_cout, k), last_target(0), target_e(0)
    {
    }

    double UpData(float feedback);
};

class FeedRotating : public FeedForward
{
  public:
    FeedRotating(float max_cout, float k) : FeedForward(max_cout, k)
    {
    }

    double UpData(float feedback);
};

class NotchFilter
{
  public:
    // 陷波滤波器结构体
    // 滤波器系数
    float a0, a1, a2;
    float b0, b1, b2;

    // 状态变量
    float x1, x2; // 输入延迟
    float y1, y2; // 输出延迟

	float fc_, fs_, Q_;
    // 初始化陷波滤波器系数
    // fc: 中心频率 (Hz)
    // fs: 采样频率 (Hz)
    // Q:  品质因数 (决定带宽)
    void initNotchFilter(float fc, float fs, float Q)
    {
        fc_ = fc;
		fs_ = fs;
		Q_ = Q;
    }

    // 处理单个采样
    float processNotchFilter(float input)
    {
        const float omega = 2.0f * 3.1415926 * fc_ / fs_;
        const float alpha = sinf(omega) / (2.0f * Q_);

        const float cos_omega = cosf(omega);

        // 计算分子系数 (零点)
        b0 = 1.0f;
        b1 = -2.0f * cos_omega;
        b2 = 1.0f;

        a0 = 1.0f + alpha;
        a1 = -2.0f * cos_omega;
        a2 = 1.0f - alpha;

        // 归一化系数
        const float inv_a0 = 1.0f / a0;
        b0 *= inv_a0;
        b1 *= inv_a0;
        b2 *= inv_a0;
        a1 *= inv_a0;
        a2 *= inv_a0;



        // 计算输出
        float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

        // 更新状态变量
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;

        return output;
    }
};


