#include "ModbusRTU.h"
#include "common/utils/ByteUtils.h"
#include "common/logger/Logger.h"

// 根据协议封装好请求帧数
QByteArray ModbusRTU::buildReadHoldingRegisters(uint8_t slaveId,
                                                uint16_t startAddr,
                                                uint16_t count)
{
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(0x03)); // FC03
    ByteUtils::appendUint16BE(frame, startAddr);
    ByteUtils::appendUint16BE(frame, count);

    QByteArray result = ByteUtils::appendCrc16(frame);
    LOG_D(QStringLiteral("TX：%1").arg(ByteUtils::toHexString(result)));
    return result;
}

// 解析发送过来的请求帧数
ModbusResponse ModbusRTU::parseReadResponse(const QByteArray &frame)
{
    ModbusResponse resp;

    LOG_D(QStringLiteral("RX(%1C)：%2")
              .arg(frame.size())
              .arg(ByteUtils::toHexString(frame)));

    // 任何有效的 Modbus RTU 响应（正常或异常）至少为 5 字节（大于7是正常的响应）
    if(frame.size() < 5)
    {
        resp.errorMsg = QStringLiteral("帧长不足：%1 bytes").arg(frame.size());
        return resp;
    }

    if(!ByteUtils::verifyCrc16(frame))
    {
        resp.errorMsg = QStringLiteral("CRC 校验失败");
        LOG_W("Modbus CRC 失败");
        return resp;
    }

    resp.slaveId = static_cast<uint8_t>(frame[0]);
    resp.funCode = static_cast<uint8_t>(frame[1]);

    // 检查异常帧（功能码最高位置 1）
    /*
     * Modbus 协议规定：
     * 从站异常时，会把功能码的最高位置 1（即 0x03 → 0x83）
     * 并在数据区第一个字节填入异常码。
     */
    // funCode = 0x03  二进制 0000 0011
    // 0000 0011 & 1000 0000 = 0000 0000 → 结果为 0（正常响应）
    // =============================================================================
    // funCode = 0x83  二进制 1000 0011
    // 1000 0011 & 1000 0000 = 1000 0000 → 结果非 0（异常响应）
    if(resp.funCode & 0x80)
    {
        // arg这样做的目的是让异常码永远以两位十六进制形式显示
        // a：待格式化的无符号整数（取自 frame[2] 的异常码）
        // fieldWidth = 2：最小显示宽度固定为 2 个字符
        // base = 16：按十六进制格式显示
        // fillChar = QLatin1Char('0')：数值不足 2 位时，前方自动补 0 填充
        resp.errorMsg = QStringLiteral("Modbus 异常，异常码：0x%1")
                            .arg(static_cast<uint8_t>(frame[2]), 2, 16, QLatin1Char('0'));
        return resp;
    }

    if(resp.funCode != 0x03)
    {
        resp.errorMsg = QStringLiteral("功能码错误：0x%1")
                            .arg(resp.funCode, 2, 16, QLatin1Char('0'));
    }

    int byteCount = static_cast<uint8_t>(frame[2]);
    // 检查帧总长度是否至少为 3（地址+功能码+字节数） + byteCount + 2（CRC）
    if(frame.size() < 3 + byteCount + 2)
    {
        resp.errorMsg = QStringLiteral("数据字节不完整");
        return resp;
    }

    // 按大端序解析寄存器值（所以是i += 2）
    // byteCount - 1 ：确保取 3 + i + 1 时不会超出数据区最后的位置
    for (int i = 0;i < byteCount - 1 ; i += 2)
    {
        uint8_t hi = static_cast<uint8_t>(frame[3 + i]);
        uint8_t lo = static_cast<uint8_t>(frame[3 + i + 1]);
        resp.registers.append(ByteUtils::toUint16BE(hi, lo));
    }

    resp.valid = true;
    return resp;
}