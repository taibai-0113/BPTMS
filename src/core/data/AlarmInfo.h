#pragma once

#include <QString>
#include <QDateTime>
#include <QMetaType>

enum class AlarmType {
    OverVoltage,
    UnderVoltage,
    OverCurrent,
    OverTemp,
    CellOverVoltage,
    CellUnderVoltage
};

struct AlarmInfo
{
    AlarmType type;
    QString typeStr;
    double actualValue {0.0};
    double threshold {0.0};
    QDateTime timestamp;
    int id {-1};
    int taskId {-1};
    int acknowledged {0};

    static QString typeToString(AlarmType t)
    {
        switch (t) {
        case AlarmType::OverVoltage:      return QStringLiteral("过压告警");
        case AlarmType::UnderVoltage:     return QStringLiteral("欠压告警");
        case AlarmType::OverCurrent:      return QStringLiteral("过流告警");
        case AlarmType::OverTemp:         return QStringLiteral("过温告警");
        case AlarmType::CellOverVoltage:  return QStringLiteral("单体过压");
        case AlarmType::CellUnderVoltage: return QStringLiteral("单体欠压");
        default: return QStringLiteral("未知告警");
        }
    }
};