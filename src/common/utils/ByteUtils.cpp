#include "ByteUtils.h"

// 字节流版 -> CRC 校验算法实现
uint16_t ByteUtils::crc16(const QByteArray &data)
{
    return crc16(reinterpret_cast<const uint8_t*>(data.constData()), data.size());
}

// 8位版 -> CRC 校验算法实现
uint16_t ByteUtils::crc16(const uint8_t *data, int len)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; ++i) {
        crc ^= static_cast<uint16_t>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
            else              crc >>= 1;
        }
    }
    return crc;
}

// 向封装好的 Modbus RTU 尾部追加CRC校验算法
QByteArray ByteUtils::appendCrc16(const QByteArray &data)
{
    uint16_t crc = crc16(data);
    QByteArray result = data;
    result.append(static_cast<char>( crc       & 0xFF)); // 低字节
    result.append(static_cast<char>((crc >> 8) & 0xFF)); // 高字节
    return result;
}

// 验证CRC是否正确
bool ByteUtils::verifyCrc16(const QByteArray &frame)
{
    // 防御性检查：去除尾部可能存在的 \r (0x0D), \n (0x0A) 或其他空白字符
    QByteArray cleanFrame = frame;
    while (!cleanFrame.isEmpty() &&
           (cleanFrame.endsWith('\r') || cleanFrame.endsWith('\n') || cleanFrame.endsWith(' '))) {
        cleanFrame.chop(1);
    }

    if (frame.size() < 4) return false;
    const int n = frame.size();
    // 提取最后两个字节：还原CRC值（低字节在前）
    uint16_t received = static_cast<uint8_t>(frame[n-2]) |
                        (static_cast<uint8_t>(frame[n-1]) << 8);
    // 计算前面数据的CRC
    uint16_t computed = crc16(reinterpret_cast<const uint8_t*>(frame.constData()), n - 2);
    return received == computed;
}

// 转十六进制字符串 -> 输入0x01 0x7E 0xC1，分隔符（sep）" " → 输出 "01 7E C1"
QString ByteUtils::toHexString(const QByteArray &data, const QString &sep)
{
    QString result;
    result.reserve(data.size() * 3); // reserve分配内存
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) result += sep;
        result += QStringLiteral("%1").arg(
                                          static_cast<uint8_t>(data[i]), 2, 16, QLatin1Char('0')).toUpper();
    }
    return result;
}

// 大端转 16 位无符号数：toUint16BE -> hi=0x01，lo=0x02 → 0x01 << 8 | 0x02 = 0x0102
uint16_t ByteUtils::toUint16BE(uint8_t hi, uint8_t lo)
{
    return (static_cast<uint16_t>(hi) << 8) | lo;
}

// 无符号转有符号 16 位数
int16_t ByteUtils::toInt16(uint16_t v)
{
    return static_cast<int16_t>(v);
}

// 追加大端 16 位数
void ByteUtils::appendUint16BE(QByteArray &arr, uint16_t value)
{
    arr.append(static_cast<char>((value >> 8) & 0xFF));
    arr.append(static_cast<char>( value       & 0xFF));
}