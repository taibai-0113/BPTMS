#include "CommWorker.h"
#include "core/device/IDevice.h"
#include "core/device/DeviceSimulator.h"
#include "core/device/BmsDevice.h"
#include "core/protocol/ModbusRTU.h"
#include "core/protocol/FrameParser.h"
#include "common/logger/Logger.h"

CommWorker::CommWorker(QObject *parent)
    : QObject(parent)
    ,m_pollTimer(new QTimer(this))

{
    // 定时器连接到onPollTick发送请求
    connect(m_pollTimer, &QTimer::timeout, this, &CommWorker::onPollTick);
}

CommWorker::~CommWorker() { stopPolling(); }

// ─── 公开槽（由主线程跨线程调用）──────────────────────
void CommWorker::connectDevice(bool simulatorMode)
{
    // 在 worker 线程中创建设备，设备天然属于 worker 线程
    if(simulatorMode)
    {
        m_device = std::make_unique<DeviceSimulator>();
        LOG_I("CommWorker：使用模拟器模式");
    }else{
        m_device = std::make_unique<BmsDevice>();
        LOG_I("CommWorker：使用真实串口");
    }
    bindDevice(m_device.get());

    if(m_device->open())
    {
        startPolling();
    }
}

void CommWorker::disconnectDevice()
{
    stopPolling();
    if(m_device)m_device->close();
}

void CommWorker::startPolling()
{
    if(!m_device || !m_device->isConnected())return;
    m_totalFrames = 0;
    m_failedFrames = 0;
    m_pollTimer->start(m_interval);
    LOG_I(QStringLiteral("CommWorker:开始轮询 interval=%1ms").arg(m_interval));
}

void CommWorker::stopPolling()
{
    m_pollTimer->stop();
    LOG_I("CommWorker：停止轮询");
}

// 数据采集时间间隔
void CommWorker::setInterval(int ms)
{
    m_interval = qMax(100, ms);
    if(m_pollTimer->isActive()) m_pollTimer->setInterval(m_interval);
}

// ─── 私有槽 ───────────────────────────────────────────
// 定时器根据用户设置的数据采集间隔发送请求
void CommWorker::onPollTick()
{
    // 请求数据 真实设备通过串口write请求 虚拟设备通过更新数据模拟器实现
    if(m_device && m_device->isConnected())m_device->requestData();
}

// 解析返回的帧
void CommWorker::onResponseReceived(QByteArray frame)
{
    // 总帧数自增
    ++m_totalFrames;

    // QByteArray ---> 转为ModbusResponse数据
    ModbusResponse resp = ModbusRTU::parseReadResponse(frame);
    if(!resp.valid)
    {
        // 失败帧自增
        ++m_failedFrames;
        emit errorOccurred(resp.errorMsg);
        emit frameStats(m_totalFrames, m_failedFrames);
        return;
    }

    // 转为ModbusResponse寄存器列表数据转为BatteryData数据供数据显示区等地方使用
    BatteryData data = FrameParser::parseBatteryData(resp.registers);
    // 回传通信质量
    emit frameStats(m_totalFrames, m_failedFrames);
    // 使用 std::move 发射，避免拷贝大结构体（体现移动语义）
    emit batteryDataReady(std::move(data));
}

// 发设备出错 关闭轮询 发送报错信息
void CommWorker::onDeviceError(QString error)
{
    LOG_E(QStringLiteral("CommWorker：%1").arg(error));

    // 判断是否为超时或普通通信错误（具体关键字根据你的底层串口报错字符串调整）
    bool isTransientError = error.contains("超时") ||
                            error.contains("Timeout") ||
                            error.contains("校验") ||
                            error.contains("CRC");

    if (isTransientError)
    {
        // 瞬态错误：仅作为失败帧处理，不停止轮询
        ++m_failedFrames;
        emit frameStats(m_totalFrames, m_failedFrames);
        emit errorOccurred(error);
    }
    else
    {
        // 致命错误：停止轮询，并可考虑在此处触发物理断开信号
        stopPolling();
        emit errorOccurred(error);
    }
}

// 发生中断 关闭轮询 发送链接断开信息
void CommWorker::onConnectionChanged(bool connected)
{
    if(!connected)stopPolling();
    emit connectionStatusChanged(connected);
}

// ─── 内部辅助 ─────────────────────────────────────────

void CommWorker::bindDevice(IDevice *dev)
{
    // Lambda 体现简介的信号槽连接写法
    connect(dev, &IDevice::responseReceived,
            this, [this](const QByteArray &f){onResponseReceived(f);});
    connect(dev, &IDevice::errorOccurred,
            this, [this](const QString &e){onDeviceError(e);});
    connect(dev, &IDevice::connectionChanged,
            this, &CommWorker::onConnectionChanged);
}