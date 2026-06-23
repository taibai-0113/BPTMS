#include "FrameParser.h"
#include "common/utils/ByteUtils.h"
#include "common/logger/Logger.h"

BatteryData FrameParser::parseBatteryData(const QList<uint16_t> &regs)
{
    BatteryData d;
    d.timestamp = QDateTime::currentDateTime();

    // 确保收到的寄存器个数至少为 17 个
    if(regs.size() < REGISTER_COUNT)
    {
        LOG_W(QStringLiteral("寄存器数量不足：%1 < %2")
                  .arg(regs.size())
                  .arg(REGISTER_COUNT));
        return d;
    }

    d.totalVoltage = regs[0] * 0.01;                           // ×0.01 V
    d.totalCurrent = ByteUtils::toInt16(regs[1]) * 0.01;       // ×0.01 A（有符号）
    d.soc          = static_cast<int>(regs[2]);
    d.soh          = static_cast<int>(regs[3]);
    d.temp1        = regs[4] * 0.1;                             // ×0.1 °C
    d.temp2        = regs[5] * 0.1;
    d.temp3        = regs[6] * 0.1;

    for(int i = 0; i < 8; ++i)d.cellVoltages[i] = regs[7 + i] * 0.001; // x0.001 V

    d.statusFlags = regs[15]; // 位运算：充电/放电/告警等标志
    d.cycleCount = static_cast<int>(regs[16]);

    return d;
}