#include "DeviceSimulator.h"
#include "common/utils/ByteUtils.h"
#include "common/logger/Logger.h"
#include "core/data/BatteryData.h"
#include <QTimer>
#include <QRandomGenerator>
#include <cmath>

DeviceSimulator::DeviceSimulator(QObject *parent) : IDevice(parent)
{
    initState();
}

DeviceSimulator::~DeviceSimulator() { DeviceSimulator::close(); }

void DeviceSimulator::initState()
{
    // 初始化 8 节单体电压（略有差异模拟真实电池不一致性）
    double base = 3.22; // 中值
    for (int i = 0; i < 8; ++i) {
        double v = base + (QRandomGenerator::global()->bounded(40) - 20) * 0.001;
        m_cellV[i] = qBound(2.8, v, 3.6);
    }
}

bool DeviceSimulator::open()
{
    m_connected = true;
    LOG_I("DeviceSimulator: 已启动");
    emit connectionChanged(true);
    return true;
}

bool DeviceSimulator::close()
{
    m_connected = false;
    LOG_I("DeviceSimulator: 已停止");
    emit connectionChanged(false);
    return true;
}

bool    DeviceSimulator::isConnected() const { return m_connected; }
QString DeviceSimulator::deviceName()  const { return QStringLiteral("BMS Simulator"); }

// 模拟真实串口情况，每5ms左右发送一次请求数据给下位机
void DeviceSimulator::requestData()
{
    if (!m_connected) return;
    // 5ms 延时模拟真实串口异步响应
    QTimer::singleShot(5, this, &DeviceSimulator::generateResponse);
}

// 模拟真实串口接收到数据（其实就是通过随机将原来的变量改变了并发送）
void DeviceSimulator::generateResponse()
{
    updateState();
    emit responseReceived(buildModbusResponse());
}

// 通过随机变量更新模拟器状态
void DeviceSimulator::updateState()
{
    m_phase += 0.05;
    auto *rng = QRandomGenerator::global();

    // 小幅随机噪声
    auto noise = [&](double amp) -> double {
        return (rng->bounded(2000) - 1000) * 0.001 * amp;
    };

    // 充放电状态切换 模拟数据起伏变化
    if (m_charging) {
        m_soc = qMin(100, m_soc + 1);
        m_current = 8.0 + noise(0.3); // 充电电流 < 30A 阈值
    } else {
        m_soc = qMax(0, m_soc - 1);
        m_current = -(10.0 + 2.0 * std::sin(m_phase)) + noise(0.2); // 放电电流 > -30A
    }

    // 总电压：将 SOC 线性映射到 [22.5, 28.3] 区间，留出噪声余量（阈值 22.4~28.5）
    m_voltage = 22.5 + (m_soc / 100.0) * 5.8 + noise(0.05);
    m_voltage = qBound(22.4, m_voltage, 28.5); // 硬限幅保证不越界

    if (m_charging && m_soc >= 100) {
        m_charging = false;
        ++m_cycleCount;
    } else if (!m_charging && m_soc <= 5) {
        m_charging = true;
    }

    // 温度更新，上限 55°C 与过温阈值一致
    for (int i = 0; i < 3; ++i) {
        m_temps[i] += noise(0.3);
        m_temps[i] = qBound(20.0, m_temps[i], 55.0);
    }

    // 单体电压：基于总电压平均分配并加小噪声，限制在 2.8~3.6V
    for (int i = 0; i < 8; ++i) {
        m_cellV[i] = m_voltage / 8.0 + noise(0.05);
        m_cellV[i] = qBound(2.8, m_cellV[i], 3.6);
    }
}


// 将模拟的数据导报成Modbus
QByteArray DeviceSimulator::buildModbusResponse() const
{
    // 按 Modbus RTU FC03 响应帧格式打包，与真实设备帧格式完全一致
    // 这样 CommWorker 可以用同一套 ModbusRTU 解析逻辑处理模拟器和真实设备

    QList<uint16_t> regs;
    // [0x0000] 总电压 ×0.01
    regs << static_cast<uint16_t>(qRound(m_voltage * 100));
    // [0x0001] 总电流 ×0.01（有符号）
    regs << static_cast<uint16_t>(static_cast<int16_t>(qRound(m_current * 100)));
    // [0x0002] SOC
    regs << static_cast<uint16_t>(m_soc);
    // [0x0003] SOH
    regs << static_cast<uint16_t>(m_soh);
    // [0x0004~0x0006] 温度 ×0.1
    for (int i = 0; i < 3; ++i)
        regs << static_cast<uint16_t>(qRound(m_temps[i] * 10));
    // [0x0007~0x000E] 单体电压 ×0.001
    for (int i = 0; i < 8; ++i)
        regs << static_cast<uint16_t>(qRound(m_cellV[i] * 1000));
    // [0x000F] 状态标志位
    uint16_t flags = m_charging ? BMS_CHARGING : BMS_DISCHARGING;
    if (m_temps[0] > 45.0 || m_temps[1] > 45.0) flags |= BMS_OVER_TEMP;
    regs << flags;
    // [0x0010] 循环次数
    regs << static_cast<uint16_t>(m_cycleCount);

    // 打包帧头
    QByteArray frame;
    frame.append(static_cast<char>(0x01));          // 从站 ID
    frame.append(static_cast<char>(0x03));          // FC03
    frame.append(static_cast<char>(regs.size() * 2)); // 字节数
    for (uint16_t r : regs)
        ByteUtils::appendUint16BE(frame, r);

    return ByteUtils::appendCrc16(frame);
}