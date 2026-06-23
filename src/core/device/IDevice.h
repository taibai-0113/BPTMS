#pragma once

#include <QObject>
// 设备抽象接口
// 真实设备（BmsDevice）与模拟器（DeviceSimulator）均继承此接口
// DeviceManager 通过 unique_ptr<IDevice>管理，上层代码对具体类型透明。
class IDevice : public QObject
{
    Q_OBJECT
public:
    explicit IDevice(QObject *parent = nullptr) : QObject(parent){}
    ~IDevice() override = default;

    // 纯虚函数
    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool isConnected() const = 0;
    virtual QString deviceName() const = 0;

public slots:
    // 请求一帧数据（异步：通过 responseReceived 发送信号）
    virtual void requestData() = 0;

signals:
    void responseReceived(QByteArray frame); // 发送数据信号
    void errorOccurred(QString errorMsg); // 发送报错信号
    void connectionChanged(bool connected); // 发哦你是个设备连接变化信号
};