#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include "core/data/AlarmConfig.h"

class AlarmConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AlarmConfigDialog(const AlarmConfig &current,
                               QWidget *parent = nullptr);
    AlarmConfig alarmConfig() const;

private:
    void setupUI();
    void loadConfig(const AlarmConfig &cfg);

    QDoubleSpinBox *m_spnOverVoltage {nullptr};
    QDoubleSpinBox *m_spnUnderVoltage {nullptr};
    QDoubleSpinBox *m_spnOverCurrent {nullptr};
    QDoubleSpinBox *m_spnOverTemp {nullptr};
    QDoubleSpinBox *m_spnCellOverVolt {nullptr};
    QDoubleSpinBox *m_spnCellUnderVolt {nullptr};
};