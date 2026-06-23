#pragma once

#include <QObject>
#include "core/data/BatteryData.h"
#include "core/data/AlarmConfig.h"
#include "core/data/AlarmInfo.h"

// 告警检测器：每帧数据到达时检查是否超过阈值
class AlarmChecker : public QObject
{
    Q_OBJECT
public:
    explicit AlarmChecker(QObject *parent = nullptr);

    void setConfig(const AlarmConfig &cfg);

public slots:
    void checkData(const BatteryData &data, int taskId = -1);

signals:
    void alarmTriggered(AlarmInfo info);

private:
    AlarmConfig m_cfg;

    void check(AlarmType type,
               double value, // 实际值
               double threshold, // 阈值
               bool isOverLimit, // 是否超出\低于阈值
               const QDateTime &ts,
               int taskId);
};