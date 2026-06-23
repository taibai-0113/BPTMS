#include "Logger.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

// 日志单例 + 日志锁 初始化
Logger *Logger::s_instance = nullptr;
QMutex Logger::s_instanceMutex;

// 单例模式，给一个唯一访问的接口，通过便捷宏实例化
Logger *Logger::instance()
{
    // 通过锁避免创建多个日志对象
    QMutexLocker lock(&s_instanceMutex);
    if(!s_instance)s_instance = new Logger();
    return s_instance;
}

// 构造函数，出生即打开当前日期的日志文件
Logger::Logger(QObject *parent) : QObject(parent)
{
    // 获取当前日期字符串
    m_currentDate = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    // 打开对应日期的日志文件
    openLogFile(m_currentDate);
}

// 关掉日志文件
Logger::~Logger()
{
    if(m_logFile.isOpen())
    {
        m_logFile.close();
    }
}

// 设置最小日志写入文件级别
void Logger::setMinLevel(LogLevel level)
{
    QMutexLocker lock(&m_mutex);
    m_minLevel = level;
}

// 对便捷宏提供接口，提供日志输出
void Logger::debug(const QString &msg, const QString &tag) {writeLog(LogLevel::DEBUG, tag, msg);}
void Logger::info(const QString &msg, const QString &tag) {writeLog(LogLevel::INFO, tag, msg);}
void Logger::warning(const QString &msg, const QString &tag) {writeLog(LogLevel::WARNING, tag, msg);}
void Logger::error(const QString &msg, const QString &tag) {writeLog(LogLevel::ERR, tag, msg);}
void Logger::critical(const QString &msg, const QString &tag) {writeLog(LogLevel::CRITICAL, tag, msg);}

// 日志写入实现，包括文件写入与控制台输出
void Logger::writeLog(LogLevel level, const QString &tag, const QString &msg)
{
    // 如果小于最小日志写入级别就不写入日志文件
    QMutexLocker lock(&m_mutex);
    if(level < m_minLevel) return;

    // 日期变化切换日志文件（按日滚动）例：2026-05-27 这就是日志文件名
    const QString today =QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    if(today != m_currentDate)
    {
        m_currentDate = today;
        openLogFile(m_currentDate);
    }

    // 获取当前时间（更精细）QDate只到年月日；QDateTime具体到当天时间
    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));
    // 把枚举值改为日志级别，如DEBUG
    const QString lvl = levelStr(level);
    /*
     * 构造最终日志行。
     * 如果 tag 为空，格式为 [时间] [级别] 消息；
     * 如果 tag 非空，格式为 [时间] [级别] [标签] 消息。
     */
    const QString line = tag.isEmpty()
                             ? QStringLiteral("[%1] [%2] %3").arg(ts, lvl, msg)
                             : QStringLiteral("[%1] [%2] [%3] %4").arg(ts, lvl, tag, msg);

    if(m_logFile.isOpen()) {
        m_stream << line << '\n';
        m_stream.flush();
    }
    qDebug().noquote() << line; // .noquote()禁止自动添加双引号

    emit logAppended(line, level);
}

// 打开文件
bool Logger::openLogFile(const QString &date)
{
    if (m_logFile.isOpen())m_logFile.close();

    // 获取项目根目录，也就是 .pro 文件所在目录 BPTMS/
    QString rootDir = QString::fromLocal8Bit(PROJECT_ROOT_DIR);

    // 获取BPTMS/logs
    QString logDir = QDir(rootDir).filePath(QStringLiteral("logs"));
    QDir().mkpath(logDir);// mkpath()创建文件

    // 拼接完整的日志
    QString logPath = QDir(logDir).filePath(date + QStringLiteral(".log"));
    m_logFile.setFileName(logPath);

    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open log file:" << logPath << m_logFile.errorString();
        return false;
    }

    // 把 m_stream 的输出目标设置到 m_logFile 这个文件上
    m_stream.setDevice(&m_logFile);
    return true;
}

// 通过日志级别返回字符串
QString Logger::levelStr(LogLevel level)
{
    switch(level)
    {
    case LogLevel::DEBUG: return QStringLiteral("DEBUG   ");
    case LogLevel::INFO: return QStringLiteral("INFO    ");
    case LogLevel::WARNING: return QStringLiteral("WARNING ");
    case LogLevel::ERR: return QStringLiteral("ERROR   ");
    case LogLevel::CRITICAL: return QStringLiteral("CRITICAL");
    default:                 return QStringLiteral("UNKNOWN ");
    }
}