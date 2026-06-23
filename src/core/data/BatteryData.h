#pragma once

#include <QDateTime>
#include <QMetaType>
#include <array>
#include <algorithm>

// Modbus 状态寄存器位定位，通过该枚举判断设备状态
enum BmsStatusBit : uint16_t {
    BMS_CHARGING      = 0x0001,
    BMS_DISCHARGING   = 0x0002,
    BMS_BALANCING     = 0x0004,
    BMS_OVER_VOLTAGE  = 0x0008,
    BMS_UNDER_VOLTAGE = 0x0010,
    BMS_OVER_TEMP     = 0x0020,
    BMS_OVER_CURRENT  = 0x0040,
    BMS_SHORT_CIRCUIT = 0x0080
};

// 实时采样数据
struct BatteryData
{
    QDateTime timestamp;

    double totalVoltage = 0.0; // 总电压
    double totalCurrent = 0.0; // 总电流
    int soc = 0; // 电池电量
    int soh = 100; // 电池健康度
    double temp1 = 0.0; // 温度采样点1
    double temp2 = 0.0;// 温度采样点2
    double temp3 = 0.0;// 温度采样点3

    // 保存 8 节单体电池电压
    std::array<double, 8>cellVoltages = {};

    // 保存 BMS 状态寄存器的值
    uint16_t statusFlags = 0;
    // 电池循环次数
    int cycleCount = 0;

    // 状态查询便捷方法
    bool isCharging() const{return statusFlags & BMS_CHARGING;}
    bool isDischarging() const{return statusFlags & BMS_DISCHARGING;}
    bool isBalancing() const{return statusFlags & BMS_BALANCING;}
    bool hasAlarm() const {return statusFlags & 0x00F8;}

    // 最大单体电压
    double maxCellVoltage() const
    {
        return *std::max_element(cellVoltages.begin(), cellVoltages.end());
    }

    // 最小单体电压
    double minCellVoltage() const
    {
        return *std::min_element(cellVoltages.begin(), cellVoltages.end());
    }

    // 最大温度
    double maxTemp() const {return std::max({temp1, temp2, temp3});}

    // 检验时间是否合法
    bool isValid() const {return timestamp.isValid();}

    // 显示声明以支持移动语义-显式声明默认构造、拷贝构造、移动构造、拷贝赋值、移动赋值和析构函数
    BatteryData() = default;
    BatteryData(const BatteryData &) = default;
    BatteryData(BatteryData &&)noexcept = default;
    BatteryData &operator=(const BatteryData &) = default;
    BatteryData &operator=(BatteryData &&)noexcept = default;
    ~BatteryData() = default;
};

// 注册为 Qt 元类型（跨线程 queued 信号传递必须）
Q_DECLARE_METATYPE(BatteryData)