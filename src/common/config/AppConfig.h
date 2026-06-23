#pragma once

#include <QMutex>
#include <QReadWriteLock>
#include "IniManager.h"
#include "JsonManager.h"
#include "core/data/AlarmConfig.h"

class AppConfig
{
public:
    static AppConfig *instance();

    // 对外提供配置的唯一接口---串口设置
    QString serialPort() const;
    int baudRate() const;
    int slaveId() const;
    bool simulatorMode() const;

    // 对外提供配置的唯一接口---虚拟设备
    void setSerialPort (const QString &p);
    void setBaudRate (int baud);
    void setSlaveId (int id);
    void setSimulatorMode (bool on);

    // 采集/图标设置
    int sampleInterval() const; // 采集数据轮询时间
    int chartTimeWindow() const; // 图表X轴可显示的最大时间（图表是滚动的）
    void setSampleInterval(int ms);

    // 告警配置
    AlarmConfig alarmConfig() const;
    void setAlarmConfig(const AlarmConfig &cfg);

    // 保存配置
    void saveIni();
    void saveAlarmConfig();

private:
    AppConfig();
    Q_DISABLE_COPY(AppConfig)

    static AppConfig *s_instance;
    static QMutex s_mutex;
    mutable QReadWriteLock m_rwLock; // 读写锁

    IniManager m_ini; // 管理ini对象
    JsonManager m_json; // 管理json对象

    static QString configDir();

    void initDefaults();

};
