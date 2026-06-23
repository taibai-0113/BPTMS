#include "AlarmChecker.h"
#include "common/logger/Logger.h"
#include <cmath>

AlarmChecker::AlarmChecker(QObject *parent) : QObject(parent){}

// 获取json中的数据，也就是用户设置的阈值
void AlarmChecker::setConfig(const AlarmConfig &cfg) { m_cfg = cfg;}

// 检查每次传输的数据是否有问题，通过设置的阈值进行比较
void AlarmChecker::checkData(const BatteryData &d, int taskId)
{
    const QDateTime &ts = d.timestamp;

    // 总电压
    check(AlarmType::OverVoltage, d.totalVoltage, m_cfg.overVoltage,
          d.totalVoltage > m_cfg.overVoltage, ts, taskId);
    check(AlarmType::UnderVoltage, d.totalVoltage, m_cfg.underVoltage,
          d.totalVoltage < m_cfg.underVoltage, ts, taskId);

    // 电流（取绝对值）
    check(AlarmType::OverVoltage, std::abs(d.totalCurrent), m_cfg.overCurrent,
          std::abs(d.totalCurrent) > m_cfg.overCurrent, ts, taskId);

    // 温度（取三路最大值）
    double maxT = d.maxTemp();
    check(AlarmType::OverTemp, maxT, m_cfg.overTemp,
          maxT>m_cfg.overTemp, ts, taskId);

    // 单体电压
    for (int i = 0; i < 8; ++i) {
        double cv = d.cellVoltages[i];
        check(AlarmType::CellOverVoltage,  cv, m_cfg.cellOverVolt,
              cv > m_cfg.cellOverVolt,  ts, taskId);
        check(AlarmType::CellUnderVoltage, cv, m_cfg.cellUnderVolt,
              cv < m_cfg.cellUnderVolt, ts, taskId);
    }
}

// 比较接口
void AlarmChecker::check(AlarmType type, double value, double threshold,
                         bool isOverLimit, const QDateTime &ts, int taskId)
{
    if(!isOverLimit)return;

    AlarmInfo info;
    info.type = type;
    info.typeStr = AlarmInfo::typeToString(type);
    info.actualValue = value;
    info.threshold = threshold;
    info.timestamp = ts;
    info.taskId = taskId;

    LOG_W(QStringLiteral("【告警】%1: 实测=%2, 阈值=%3")
              .arg(info.typeStr)
              .arg(value, 0, 'f', 3)
              .arg(threshold, 0, 'f', 3));

    emit alarmTriggered(info);
}