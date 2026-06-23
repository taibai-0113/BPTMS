#pragma once

#include <QObject>
#include <QSerialPort>
#include <QTimer>

// 串口配置 8N1
struct SerialConfig
{
    QString portName = "COM3";
    int baudRate = 9600;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits  = QSerialPort::OneStop;
    int timeoutMs = 500;
};

class SerialPortManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager() override;

    // 串口管理基本函数
    bool open(const SerialConfig &cfg);
    bool close();
    bool isOpen()const;

    bool write(const QByteArray &data);

    // 遍历设备COM口
    static QStringList availablePorts();

signals:
    void dataReceived (QByteArray data); // 发送串口接受到的数据信号
    void errorOccurred (QString error);

private slots:
    void onReadyRead(); // 接受串口数据
    void onSerialError(QSerialPort::SerialPortError error); // 串口报错
    void onResponseTimeout(); // 串口连接超时

private:
    QSerialPort *m_serial;
    QTimer *m_timeout; // 检查串口是否连接超时
    QByteArray m_rxBuffer; // 缓冲池
    int m_expectedLen; // 设置完整帧数字节数量
};