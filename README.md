



# DBUS_Test



# Motor_test



# DWT_test

## 简介
​	实现了一个基于DWT（Data Watchpoint and Trace）外设的高精度定时器类 `DWTimer`，用于测量时间间隔和提供延迟功能。项目使用了C++11特性，并通过单例模式封装了定时器的功能，便于在嵌入式系统中进行时间相关的操作。

---

## 文件结构

### 1. `DWT.hpp`
- **功能**: 定义了 `DWTimer` 类及其相关方法。
- **主要特性**:
  - 使用单例模式管理 `DWTimer` 实例。
  - 提供时间差计算、时间轴获取和延时功能。
  - 内部使用原子变量 (`std::atomic`) 确保多线程环境下的安全性。
  - 支持构造函数私有化，防止外部直接实例化。
- **关键方法**:
  - `GetInstance`: 获取 `DWTimer` 单例实例。
  - `GetDeltaT`: 计算两次调用之间的时间差（32位精度）。
  - `GetDeltaT64`: 计算两次调用之间的时间差（64位精度）。
  - `GetTimeline_s`: 获取当前时间轴（秒级）。
  - `GetTimeline_ms`: 获取当前时间轴（毫秒级）。
  - `GetTimeline_us`: 获取当前时间轴（微秒级）。
  - `Delay`: 提供精确的延迟功能。

### 2. `DWT.cpp`
- **功能**: 实现了 `DWT.hpp` 中定义的 `DWTimer` 类的方法。
- **主要特性**:
  - 初始化 DWT 外设并配置 CYCCNT 寄存器。
  - 更新 CYCCNT 计数器以支持溢出处理。
  - 提供多种时间单位的转换逻辑。
- **关键方法实现**:
  - `Init`: 配置 DWT 外设并初始化 CPU 频率相关参数。
  - `UpdateCYCCNT`: 检测 CYCCNT 溢出并更新计数器。
  - `UpdateSysTime`: 将 CYCCNT 转换为秒、毫秒和微秒的时间格式。

### 3. `CallBack.cpp`
- **功能**: 实现了回调函数和测试逻辑。
- **主要特性**:
  - 使用CAN板并用 HAL 库生成的 TIM7 中断回调函数来测量时间间隔。
  - 提供了一个 `in_while` 函数用于测试在主循环里代码段的执行时间。
- **关键变量**:
  - `ins_dt`: 存储中断回调中的时间差。
  - `while_dt`: 存储 `in_while` 函数中的时间差。
  - `last_count` 和 `while_count`: 用于记录上一次的时间戳。

---

## 使用说明

### 1. 初始化
在 `CallBack.cpp` 的 `Init` 函数中，初始化 TIM7 中断并获取 `DWTimer` 单例实例：
```cpp
void Init()
{
    HAL_TIM_Base_Start_IT(&htim7);
    auto &timer = BSP::DWTimer::GetInstance(168); // 获取 DWTimer 实例，CPU 频率为 168 MHz
}
```

> 在初始化函数里填写构造函数参数后，后续就不用了

### 2.测量时间差
在 TIM7 的中断回调函数中，调用 `GetDeltaT` 方法测量时间差：

~~~c++
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM7)
    {
        auto &timer = BSP::DWTimer::GetInstance();
        callbcak_dt = timer.GetDeltaT(&last_count); // 测量时间差
    }
}
~~~

> cnt_last需要是全局变量或者用static修饰

### 3. 获取时间轴

获取当前程序运行的时间，只需调用以下函数就行：

~~~c++
auto &timer = BSP::DWTimer::GetInstance();

float seconds = timer.GetTimeline_s(); // 获取秒级时间轴
float milliseconds = timer.GetTimeline_ms(); // 获取毫秒级时间轴
uint64_t microseconds = timer.GetTimeline_us(); // 获取微秒级时间轴
~~~

### 4.延时功能

利用dwt，可以实现更加精确的时间延时：

```c++
auto &timer = BSP::DWTimer::GetInstance();
timer.Delay(0.0001f); // 延时 0.0001 秒
```
---
## 注意事项

### 1.CPU频率设置

- 在创建`DWTimer`实例时，需要指定正确的 CPU 频率（单位为 MHz）。例如：`BSP::DWTimer::GetInstance(168)` 表示 CPU 频率为 168 MHz。

- `GetDeltaT` 和 `GetTimeline` 方法会更新内部计数器，频繁调用可能对性能产生一定影响。
# Dji Mini遥控器

---

## 简介

​	用于解析新版RM遥控器数据的c++库，由于是以Dji Mini无人机遥控器的外观，所以命名为RemoteMini

> 使用部分c++17语法如嵌套namespace

​	使用位域结构体,将数据直接复制进结构体,再调用`UpdateStatus`进行*归一化处理*

​	通过单例模式，省去extern的麻烦

## 文件结构

```
RemoteMini_test/
├── User/
│   ├── MimiRemote/
│   │   ├── Mini.hpp      # 头文件，定义了 Mini 类及其相关数据结构
│   │   └── Mini.cpp      # 实现文件，包含 Mini 类方法的具体实现
│	├── Key/
│	│	├── KeyBorad.hpp  # only heard库，实现简单的按键检测
│   └── CRC/              # CRC 校验相关代码
└── README.md             # 项目说明文档
```

## 使用说明

### 1. 初始化

​	在程序启动时调用`Init`方法初始化串口DMA接收:

```c++
    // 获取实例
    auto &remote = BSP::Remote::Mini::getInstance();
    remote.Init();
```

### 2. 数据解析

​	当串口受到数据后,在**HAL_UARTEx_RxEventCallback函数**中调用`Parse`方法进行数据解析:

```c++
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    auto &remote = BSP::Remote::Mini::getInstance();

    remote.Parse(huart, Size);
}

```

### 3. 获取遥控器状态

​	通过`get`方法获取遥控器状态,使用方法如下

#### 1. 遥感值

```c++
auto rightJoystick = remote.remoteRight(); // 右侧摇杆
auto leftJoystick = remote.remoteLeft();   // 左侧摇杆
```

#### 2. 开关状态

```c++
auto gear = remote.gear();       // 挡位开关
auto paused = remote.paused();   // 暂停按键
auto fnLeft = remote.fnLeft();   // 左侧自定义按键
auto fnRight = remote.fnRight(); // 右侧自定义按键
auto trigger = remote.trigger(); // 扳机键
```

#### 3. 鼠标状态

```c++
auto mouseVelocity = remote.mouseVel(); // 鼠标速度
auto mouseKeys = remote.mouse();        // 鼠标按键
```

#### 4. 键盘状态

```c++
auto keyboard = remote.keyBoard(); // 键盘按键
```

---

## SimpleKey 按键检测类

### 功能简介
SimpleKey 是一个简单而功能完整的按键检测类，用于处理单个按键的各种输入状态。支持以下功能：
- 点击检测（短按）
- 长按检测
- 开关状态（Toggle）

### 特性
- 点击和长按互斥
- 支持开关模式（每次点击切换状态）

### 使用方法

#### 1. 基本使用
```cpp
// 创建实例
BSP::Key::SimpleKey key;

// 在主循环中更新按键状态
void loop() {
    key.update(/* 按键输入值 */);  // 传入当前按键的状态（true为按下，false为释放）
}
```

#### 2. 状态检测
```cpp
// 检测点击（短按）
if (key.getClick()) {
    // 处理点击事件
    // 注意：getClick() 读取后会自动清零
}

// 检测长按
if (key.getLongPress()) {
    // 处理长按事件
}

// 获取开关状态
if (key.getToggleState()) {
    // 处理开关打开状态
}
```

### 参数配置
- 长按判定阈值：`LONG_PRESS_THRESHOLD = 500ms`
  - 可以通过修改此值来调整长按触发的时间

### 工作原理

#### 状态检测逻辑
1. **按下瞬间**：
   - 记录按下时刻
   - 重置长按检测标志
   - 切换开关状态

2. **按住状态**：
   - 持续检测是否达到长按时间阈值
   - 达到阈值后触发长按状态

3. **释放瞬间**：
   - 如果未达到长按时间，则判定为点击
   - 重置长按检测标志

#### 状态互斥
- 长按和点击是互斥的，不会同时触发
- 长按触发后不会产生点击事件

### 注意事项
1. `getClick()` 是一次性读取，读取后会自动清零
2. 长按状态会持续到按键释放
3. 开关状态在每次点击时切换，不受长按影响
4. 建议在主循环中定期调用 update() 更新按键状态
