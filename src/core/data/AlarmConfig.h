#pragma once

#include <QJsonObject>

// 警告阈值结构体，定义上限
struct AlarmConfig
{
    double overVoltage = 28.5; // 过压阈值
    double underVoltage = 22.4; // 低压阈值
    double overCurrent = 30.0; // 过流阈值
    double overTemp = 55.0; // 过温阈值
    double cellOverVolt = 3.6; // 单体过压阈值
    double cellUnderVolt = 2.8; // 单体低压阈值

    // QJsonObject ---> Json
    QJsonObject toJson() const
    {
        return {
            {"overVoltage",   overVoltage},
            {"underVoltage",  underVoltage},
            {"overCurrent",   overCurrent},
            {"overTemp",      overTemp},
            {"cellOverVolt",  cellOverVolt},
            {"cellUnderVolt", cellUnderVolt}
        };
    }

    // Json ---> AlarmConfig
    static AlarmConfig fromJson(const QJsonObject &o)
    {
        AlarmConfig c;
        // toDouble(c.overVoltage) 里的参数：这是转换失败时返回的默认值
        if (o.contains("overVoltage"))   c.overVoltage   = o["overVoltage"].toDouble(c.overVoltage);
        if (o.contains("underVoltage"))  c.underVoltage  = o["underVoltage"].toDouble(c.underVoltage);
        if (o.contains("overCurrent"))   c.overCurrent   = o["overCurrent"].toDouble(c.overCurrent);
        if (o.contains("overTemp"))      c.overTemp      = o["overTemp"].toDouble(c.overTemp);
        if (o.contains("cellOverVolt"))  c.cellOverVolt  = o["cellOverVolt"].toDouble(c.cellOverVolt);
        if (o.contains("cellUnderVolt")) c.cellUnderVolt = o["cellUnderVolt"].toDouble(c.cellUnderVolt);
        return c;
    }
};