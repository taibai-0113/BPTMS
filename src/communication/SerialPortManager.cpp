#include "SerialPortManager.h"
#include "common/logger/Logger.h"
#include <QSerialPort>
#include <QSerialPortInfo>

// 串口管理类初始化并建立信号槽
SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
    , m_timeout(new QTimer(this))
    , m_expectedLen(39)// 一条完整的应答帧恰好为 39 字节
{
    m_timeout->setSingleShot(true); // 单次定时

    // 串口信号槽
    connect(m_serial,  &QSerialPort::readyRead,
            this,      &SerialPortManager::onReadyRead);
    connect(m_serial,  &QSerialPort::errorOccurred,
            this,      &SerialPortManager::onSerialError);
    connect(m_timeout, &QTimer::timeout,
            this,      &SerialPortManager::onResponseTimeout);
}

SerialPortManager::~SerialPortManager() {close();}

// 打开串口并初始化串口配置
bool SerialPortManager::open(const SerialConfig &cfg)
{
    if(m_serial->isOpen()) m_serial->close();
    m_serial->setPortName(cfg.portName);
    m_serial->setBaudRate(cfg.baudRate);
    m_serial->setDataBits(cfg.dataBits);
    m_serial->setParity(cfg.parity);
    m_serial->setStopBits(cfg.stopBits);

    if(!m_serial->open(QIODevice::ReadWrite))
    {
        LOG_E(QStringLiteral("串口打开失败：%1 -> %2")
              .arg(cfg.portName, m_serial->errorString()));
        emit errorOccurred(m_serial->errorString());
        return false;
    }

    LOG_I(QStringLiteral("串口已打开：%1 @ %2bps")
              .arg(cfg.portName, QString::number(cfg.baudRate)));

    // 清空数据缓冲池
    m_rxBuffer.clear();
    return true;
}

// 关闭串口
bool SerialPortManager::close()
{
    if(!m_serial->isOpen())return false;
    m_serial->close();
    LOG_I("串口已关闭");
    return true;
}

// 判断串口是否打开
bool SerialPortManager::isOpen() const {return m_serial->isOpen();}

// 通过串口发送数据
bool SerialPortManager::write(const QByteArray &data)
{
    if(!m_serial->isOpen())return false;
    m_rxBuffer.clear();
    m_serial->write(data);
    m_timeout->start(500);
    return true;
}

// 遍历本机所有串口并放置到List中
QStringList SerialPortManager::availablePorts()
{
    QStringList result;
    for(const auto &info : QSerialPortInfo::availablePorts())
        result<<info.portName();
    return result;
}

// 通过串口接收到的数据写入m_rxBuffer中
void SerialPortManager::onReadyRead()
{
    m_rxBuffer.append(m_serial->readAll());
    if(m_rxBuffer.size() >= m_expectedLen)
    {
        m_timeout->stop();
        emit dataReceived(m_rxBuffer);
        m_rxBuffer.clear();
    }
}

// 串口错误槽函数
void SerialPortManager::onSerialError(QSerialPort::SerialPortError err)
{
    if(err != QSerialPort::NoError)emit errorOccurred(m_serial->errorString());
}

// 响应超时槽函数
void SerialPortManager::onResponseTimeout()
{
    if(!m_rxBuffer.isEmpty())
    {
        // 收到数据了，但是超时了，交给上层判断
        emit dataReceived(m_rxBuffer);
        m_rxBuffer.clear();
    }else{
        emit errorOccurred(QStringLiteral("响应超时"));
    }
}