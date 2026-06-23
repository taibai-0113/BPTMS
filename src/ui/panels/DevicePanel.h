#pragma once

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>

class DevicePanel : public QWidget
{
    Q_OBJECT
public:
    explicit DevicePanel(QWidget *parent = nullptr);

    void setConnected(bool connected, const QString &deviceName = {});

signals:
    void connectRequested(bool simulatorMode, const QString &port, int baud);
    void disconnectRequested();
    void intervalChanged(int ms); // 发送信息采集间隔

private slots:
    void refreshPorts(); // 刷新串口信号

public slots:
    void onConnectClicked(); // 发送设备连接信号跟设备基本信息
    void updateFrameStats(int total, int failed); // 更新通信质量状态

private:
    void setupUI();

    QComboBox *m_cbPort; // 选择串口
    QComboBox *m_cbBaud; // 选择波特率
    QCheckBox *m_chkSimulator; // 选择是否使用模拟器
    QPushButton *m_btnConnect; // 连接按钮
    QPushButton *m_btnDisconnect; // 断开按钮
    QPushButton *m_btnRefresh; // 刷新按钮（刷新串口）
    QLabel *m_lblStatus; // 设备状态
    QSpinBox *m_spnInterval; // 采样间隔
    QLabel *m_lblFrameCount; // 帧计数
    QLabel *m_lblQuality; // 通信质量
    QLabel *m_lblRunTime; // 运行时长

    bool m_connected {false}; // 连接状态
    int m_elapsed {0}; // 记录从开始采集/连接到现在已经过了多少秒
    QTimer *m_runTimer; // 定时器    
};