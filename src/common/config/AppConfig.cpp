#include "AppConfig.h"
#include <QDir>

AppConfig *AppConfig::s_instance = nullptr;
QMutex AppConfig::s_mutex;

// 单例模式
AppConfig *AppConfig::instance()
{
    // 加锁
    QMutexLocker lock(&s_mutex);
    if(!s_instance) s_instance = new AppConfig();
    return s_instance;
}

// 获取永久化配置根路径
QString AppConfig::configDir()
{
    QString rootDir = QString::fromLocal8Bit(PROJECT_ROOT_DIR);
    QString dir = QDir(rootDir).filePath(QStringLiteral("config"));
    QDir().mkpath(dir);
    return dir;
}

// 加载配置
AppConfig::AppConfig()
    : m_ini(QDir(configDir()).filePath("app.ini")),
    m_json(QDir(configDir()).filePath("alarm_config.json"))
{
    initDefaults();
}

// 初始化ini跟json
void AppConfig::initDefaults()
{
    if(!m_ini.contains("Serial/port"))
    {
        m_ini.set("Serial/port", QStringLiteral("COM3"));
        m_ini.set("Serial/baudRate", 9600);
        m_ini.set("Serial/slaveId", 1);
        m_ini.set("Serial/simulatorMode", true);
        m_ini.set("Polling/interval", 1000);
        m_ini.set("Chart/timeWindow", 60);
    }

    if(m_json.isEmpty())
    {
        m_json.setRoot(AlarmConfig{}.toJson());
        m_json.save();
    }
}

// 获取ini中的配置 (读操作， QReadLocker)
QString AppConfig::serialPort() const{QReadLocker locker(&m_rwLock); return m_ini.get<QString>("Serial/port", "COM3");}
int AppConfig::baudRate() const{QReadLocker locker(&m_rwLock); return m_ini.get<int>("Serial/baudRate", 9600);}
int AppConfig::slaveId() const{QReadLocker locker(&m_rwLock); return m_ini.get<int>("Serial/slaveId", 1);}
bool AppConfig::simulatorMode() const{QReadLocker locker(&m_rwLock); return m_ini.get<bool>("Serial/simulatorMode", true);}
int AppConfig::sampleInterval() const{QReadLocker locker(&m_rwLock); return m_ini.get<int>("Polling/interval", 1000);}
int AppConfig::chartTimeWindow() const{QReadLocker locker(&m_rwLock); return m_ini.get<int>("Chart/timeWindow", 60);}

// 放置ini中的配置 (写操作， QWriteLocker)
void AppConfig::setSerialPort(const QString &p){QWriteLocker locker(&m_rwLock); m_ini.set("Serial/port", p);}
void AppConfig::setBaudRate(int b){QWriteLocker locker(&m_rwLock); m_ini.set("Serial/baudRate", b);}
void AppConfig::setSlaveId(int id){QWriteLocker locker(&m_rwLock); m_ini.set("Serial/slaveId", id);}
void AppConfig::setSimulatorMode(bool on){QWriteLocker locker(&m_rwLock); m_ini.set("Serial/simulatorMode", on);}
void AppConfig::setSampleInterval(int ms){QWriteLocker locker(&m_rwLock); m_ini.set("Polling/interval", ms);}

// 将json解析到结构体中并返回该结构体 (读操作， QReadLocker)
AlarmConfig AppConfig::alarmConfig()const
{
    QReadLocker locker(&m_rwLock);
    return AlarmConfig::fromJson(m_json.root());
}

// 放置最新的json到文件中（最新阈值） (写操作， QWriteLocker)
void AppConfig::setAlarmConfig(const AlarmConfig &cfg)
{
    QWriteLocker locker(&m_rwLock);
    m_json.setRoot(cfg.toJson());
    m_json.save();
}

// 强制写入与保存 (写操作，涉及底层文件IO与内部状态同步，写操作， QWriteLocker)
void AppConfig::saveIni()
{
    QWriteLocker locker(&m_rwLock);
    m_ini.sync();
}

void AppConfig::saveAlarmConfig()
{
    QWriteLocker locker(&m_rwLock);
    m_json.save();
}