#pragma once

#include <QObject>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

// 枚举日志级别 每个日志标识代表的含义
enum class LogLevel { DEBUG=0, INFO, WARNING, ERR, CRITICAL};

// 日志单例模式
class Logger : public QObject
{
    Q_OBJECT
public:
    // 全局唯一的访问点，返回Logger对象的静态指针
    static Logger *instance();

    // 设置最小阈值，低于此级别的日志不会被写入文件或发送信号
    void setMinLevel(LogLevel level);

    // 对外公共接口，提供日志输出，但是最终写入依靠的是writeLog函数
    void debug (const QString &msg, const QString &tag = {});
    void info (const QString &msg, const QString &tag = {});
    void warning (const QString &msg, const QString &tag = {});
    void error (const QString &msg, const QString &tag = {});
    void critical (const QString &msg, const QString &tag = {});

signals:
    // 写入日志时自动发射此信号
    void logAppended(QString formattedLine, LogLevel level);

private:
    // 通过QT的对象树机制自动管理内存
    explicit Logger(QObject *parent = nullptr);
    ~Logger()override;

    /*
     * 禁用深拷贝跟运算符重载，相当于👇
     * Logger(const Logger &) = delete;
     * Logger &operator=(const Logger &) = delete;
     */
    Q_DISABLE_COPY(Logger)

    // 日志对象指针
    static Logger *s_instance;
    // 防止多个线程同时创建日志导致出现问题而加入的锁
    static QMutex s_instanceMutex;

    QMutex m_mutex; // 防止多个线程同时读取日志导致出现问题而加入的锁
    QFile m_logFile; // 日志文件QFile对象
    QTextStream m_stream; // 数据写入流，中转站
    LogLevel m_minLevel {LogLevel::DEBUG}; // 最低日志写入级别
    QString m_currentDate; // 时间

    // 打印日志
    void writeLog(LogLevel level, const QString &tag, const QString &msg);
    // 打开日志文件
    bool openLogFile(const QString &date);
    // 获取日志级别对应的字符串
    static QString levelStr(LogLevel level);
};

// 便捷宏（自动带函数名 tag）
// QLatin1String 是一个轻量级字符串开销很小，并且可以避免因为字符编码而带来的问题
// __FUNCTION__ 可以在编译时被替换为当前函数的函数名
// 所以QLatin1String(__FUNCTION__)的作用是返回当前函数名的QLatin1String对象
// 问题：QLatin1String既然这么轻量级以及可以避免问题，为什么还要用QString
// 答：因为QLatin1String仅支持Latin-1，不支持其他字符（比如中文、韩文等0），并且其不分配内存、只读
#define LOG_D(msg) Logger::instance()->debug (msg, QLatin1String(__FUNCTION__))
#define LOG_I(msg) Logger::instance()->info (msg, QLatin1String(__FUNCTION__))
#define LOG_W(msg) Logger::instance()->warning (msg, QLatin1String(__FUNCTION__))
#define LOG_E(msg) Logger::instance()->error (msg, QLatin1String(__FUNCTION__))