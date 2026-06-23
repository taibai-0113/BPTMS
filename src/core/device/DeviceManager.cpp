#include "DeviceManager.h"
#include "DeviceSimulator.h"
#include "BmsDevice.h"
#include "common/logger/Logger.h"

DeviceManager::DeviceManager(QObject *parent) : QObject(parent) {}
DeviceManager::~DeviceManager() = default;

// 创建新的模拟器
void DeviceManager::createSimulator()
{
    if (m_device) m_device->close();
    m_device = std::make_unique<DeviceSimulator>();
    LOG_I("DeviceManager: 已创建模拟器");
    // m_device.get() ---> 手动取出内部的原始指针，专门用于传参
    emit deviceChanged(m_device.get());
}

// 创建新的真实设备
void DeviceManager::createRealDevice()
{
    if (m_device) m_device->close();
    m_device = std::make_unique<BmsDevice>();
    LOG_I("DeviceManager: 已创建真实设备");
    emit deviceChanged(m_device.get());
}

// 获得真实\虚拟设备 判断真实\虚拟设备是否存在
IDevice *DeviceManager::currentDevice() const { return m_device.get();      }
bool     DeviceManager::hasDevice()     const { return m_device != nullptr;  }