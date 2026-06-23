#pragma once

#include "IDevice.h"
#include "communication/SerialPortManager.h"
#include <memory>

// 真实设备
class BmsDevice : public IDevice
{
    Q_OBJECT
public:
    explicit BmsDevice(QObject *parent = nullptr);
    ~BmsDevice() override;

    // 实现基本函数
    bool open() override;
    bool close() override;
    bool isConnected() const override;
    QString deviceName() const override;

public slots:
    void requestData() override;

private slots:
    void onDataReceived(const QByteArray &data);
    void onError(const QString &error);

private:
    // 使用独占指针体现智能指针所有权语义
    std::unique_ptr<SerialPortManager> m_serial; // 通过串口收发数据
    bool m_connected {false};
    int m_slaveId {1};

    static constexpr uint16_t MODBUS_START_ADDR = 0X0000;
    static constexpr uint16_t MODBUS_REG_COUNT = 0X00011; // 十进制的17
};