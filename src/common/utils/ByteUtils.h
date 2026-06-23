#pragma once

#include <QByteArray>
#include <QString>
#include <cstdint>

class ByteUtils
{
public:
    ByteUtils() = delete;

    // CRC16-Modbus 标准实现
    static uint16_t crc16(const QByteArray &data);
    static uint16_t crc16(const uint8_t *data, int len);

    // 在帧末尾追加 CRC（低字节在前，符合 Modbus RTU 规范）
    static QByteArray appendCrc16(const QByteArray &data);

    // 验证末尾 2 字节 CRC
    static bool verifyCrc16(const QByteArray &frame);

    // 字节数组 → 十六进制字符串（调试专用）
    static QString toHexString(const QByteArray &data, const QString &sep = QStringLiteral(" "));

    // 大端序 uint16（高字节在前）
    static uint16_t toUint16BE(uint8_t hi, uint8_t lo);

    // uint16 → int16（有符号转换）
    static int16_t  toInt16(uint16_t v);

    // 追加 uint16 大端序到 QByteArray
    static void appendUint16BE(QByteArray &arr, uint16_t value);
};