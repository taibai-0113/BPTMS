#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <QList>
#include <cstdint>
#include "core/data/BatteryData.h"

class FrameParser
{
public:
    FrameParser() = delete;
    static constexpr int REGISTER_COUNT = 17; // 0x0000 ~ 0x0010

    // 17 个寄存器 -> BatteryData （含比例因子换算 + 位运算解析状态）
    static BatteryData parseBatteryData(const QList<uint16_t> &registers);
};

#endif // FRAMEPARSER_H
