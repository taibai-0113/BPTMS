#pragma once

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

class DeviceConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DeviceConfigDialog(QWidget *parent = nullptr);

private slots:
    void onAccept();

private:
    void setupUI();

    QComboBox *m_cbPort {nullptr};
    QComboBox *m_cbBaud {nullptr};
    QSpinBox *m_spnSlave {nullptr};
    QCheckBox *m_chkSim {nullptr};
    QSpinBox *m_spnInterval {nullptr};
};