#include "BmsDevice.h"
#include "core/protocol/ModbusRTU.h"
#include "common/config/AppConfig.h"
// #include "common/logger/Logger.h"

BmsDevice::BmsDevice(QObject *parent)
    :IDevice(parent)
    ,m_serial(std::make_unique<SerialPortManager>())
{
    // 接受来自串口的数据，实现信号槽
    connect(m_serial.get(), &SerialPortManager::dataReceived,
            this, &BmsDevice::onDataReceived);
    connect(m_serial.get(), &SerialPortManager::errorOccurred,
            this, &BmsDevice::onError);
}

BmsDevice::~BmsDevice(){BmsDevice::close();}

// 真实设备打开，将ini中的串口配置通过这个open函数传入到SerialConfig中
bool BmsDevice::open()
{
    AppConfig *cfg = AppConfig::instance();
    SerialConfig sc;
    sc.portName = cfg->serialPort();
    sc.baudRate = cfg->baudRate();
    m_slaveId = cfg->slaveId();

    m_connected = m_serial->open(sc);
    emit connectionChanged(m_connected); // 虽然头文件中没有声明信号，但是IDevice有
    return m_connected;
}

// 关闭串口
bool BmsDevice::close()
{
    m_connected = false;
    m_serial->close();
    emit connectionChanged(false);
    return true;
}

// 返回连接状态跟设备名称
bool BmsDevice::isConnected() const {return m_connected;}
QString BmsDevice::deviceName() const {return QStringLiteral(" BMS Real Device");}

// 公共槽函数，当真实设备被用户选择连接时会触发轮询
void BmsDevice::requestData()
{
    if (!m_connected) return;
    // 构造请求帧数 --- 从站地址 + 功能码 + 寄存器数量
    QByteArray req = ModbusRTU::buildReadHoldingRegisters(
        m_slaveId, MODBUS_START_ADDR, MODBUS_REG_COUNT);
    m_serial->write(req);
}

void BmsDevice::onDataReceived(const QByteArray &data)
{
    emit responseReceived(data); // 继续接力传递数据
}

void BmsDevice::onError(const QString &error)
{
    // 判断是否为瞬态错误（超时、CRC 等），瞬态错误不改变连接状态
    bool isTransient = error.contains("超时") ||
                       error.contains("Timeout", Qt::CaseInsensitive) ||
                       error.contains("校验") ||
                       error.contains("CRC", Qt::CaseInsensitive);

    if (!isTransient)
    {
        // 致命错误（比如串口被拔出、设备不存在等）才真正断开
        m_connected = false;
        emit connectionChanged(false);
    }

    emit errorOccurred(error); // 始终上报错误供 CommWorker 统计失败帧
}