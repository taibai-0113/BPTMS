#pragma once

#include "IDevice.h"
#include <array>

class DeviceSimulator : public IDevice
{
    Q_OBJECT
public:
    explicit DeviceSimulator(QObject *parent = nullptr);
    ~DeviceSimulator() override;

    bool    open()             override;
    bool    close()            override;
    bool    isConnected() const override;
    QString deviceName()  const override;

public slots:
    void requestData() override; // 模拟器请求数据

private slots:
    void generateResponse(); // 模拟数据变化并发送模拟后的数据信号

private:
    bool m_connected { false };

    // ── 仿真电池内部状态 ──────────────────────────────
    double m_voltage     { 48.0  };
    double m_current     { -10.0 };
    int    m_soc         { 80    };
    int    m_soh         { 96    };
    double m_temps[3]    { 28.0, 29.5, 27.5 };
    std::array<double,8> m_cellV { };
    int    m_cycleCount  { 42   };
    bool   m_charging    { false };
    double m_phase       { 0.0  };

    void       initState();
    void       updateState();
    QByteArray buildModbusResponse() const;  // 打包为标准 Modbus RTU 响应帧
};