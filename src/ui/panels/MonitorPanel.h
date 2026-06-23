#pragma once

#include <QWidget>
#include <QLabel>
#include <QTabWidget>
#include "core/data/BatteryData.h"
#include "core/data/AlarmConfig.h"
#include "ui/widgets/ScrollingChart.h"

class DataCard;
class CellVoltageBar;
class ScrollingChart;

class MonitorPanel : public QWidget
{
    Q_OBJECT
public:
    explicit MonitorPanel(QWidget *parent = nullptr);

    void updateData(const BatteryData &data);
    void clearDisplay();
    void setAlarmConfig(const AlarmConfig &cfg);

public slots:
    void setTimeWindow(int ms);

private:
    void setupUI();
    // void setupCards();
    // void setupStatusArea();

    // 数值卡片
    DataCard *m_cardVoltage {nullptr};
    DataCard *m_cardCurrent {nullptr};
    DataCard *m_cardSOC {nullptr};
    DataCard *m_cardSOH {nullptr};
    DataCard *m_cardTemp1 {nullptr};
    DataCard *m_cardTemp2 {nullptr};
    DataCard *m_cardTemp3 {nullptr};
    DataCard *m_cardPower {nullptr};
    DataCard *m_cardCycles {nullptr};

    CellVoltageBar *m_cellBar {nullptr};

    // 折线图
    ScrollingChart *m_chartVoltage  { nullptr };
    ScrollingChart *m_chartCurrent  { nullptr };
    ScrollingChart *m_chartSOC      { nullptr };
    ScrollingChart *m_chartTemp     { nullptr };
    QTabWidget     *m_chartTab      { nullptr };

    // 状态指示
    QLabel *m_lblChargeStatus {nullptr};
    QLabel *m_lblAlarmBanner {nullptr};
    QLabel *m_lblTimestamp {nullptr};

    AlarmConfig m_cfg;
};