#pragma once

#include <QObject>
#include <QTimer>
#include <memory>
#include "core/data/BatteryData.h"

class IDevice;

// ─── 通信 Worker ───────────────────────────────────────
// 生命周期：由 MainWindow 创建后 moveToThread 到 worker 线程
// 设备对象(独占指针)在 connectDevice() 槽中创建，自动归属于 worker 线程
class CommWorker : public QObject{
    Q_OBJECT
public:
    explicit CommWorker(QObject *parent = nullptr);
    ~CommWorker() override;

public slots:
    // 由主线程通过信号槽调用（自动queued）
    void connectDevice(bool simulatorMode); // 链接设备
    void disconnectDevice(); // 断开设备
    void startPolling(); // 启动轮询
    void stopPolling(); // 停止轮询
    void setInterval(int ms); // 轮询间隔（数据采样间隔）

signals:
    void batteryDataReady(BatteryData data); // UI刷新 / DB写入
    void errorOccurred(QString error);
    void connectionStatusChanged(bool connected);
    void frameStats(int total, int failed); // 状态栏显示的通信质量

private slots:
    void onPollTick();
    void onResponseReceived(QByteArray frame);
    void onDeviceError(QString error);
    void onConnectionChanged(bool connected);

private:
    QTimer *m_pollTimer; // 定时器
    std::unique_ptr<IDevice> m_device; // Worker 线程独占设备所有权，真正管理设备的地方

    int m_interval {1000}; // 每隔多少毫秒触发
    int m_totalFrames {0}; // 总帧数
    int m_failedFrames{0}; // 失败帧数

    void bindDevice(IDevice *device);
};