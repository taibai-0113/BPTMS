#pragma once

#include "IDevice.h"
#include <memory>

// DeviceManager 通过 unique_ptr 独占设备对象生命周期
// 上层通过 currentDevice() 获得裸指针（non-owning）进行操作
class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager() override;

    // 创建真实\虚拟设备
    void createSimulator();
    void createRealDevice();

    IDevice *currentDevice() const;
    bool     hasDevice()     const;

signals:
    void deviceChanged(IDevice *newDevice); // 真实\虚拟设别如果切换了话就返回新的设备

private:
    std::unique_ptr<IDevice> m_device; // 统一管理真实\虚拟设备（这里用的是独占指针）
};