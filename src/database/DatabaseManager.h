#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QMutex>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager *instance();

    bool init(const QString &path = "bptms.db"); // 初始化数据库
    bool isOpen() const; // 数据库是否打开
    QSqlDatabase database() const; // 返回数据库对象

    // 执行任意 SQL （建表/维护专用）
    bool execSQL(const QString &sql);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(DatabaseManager)

    static DatabaseManager *s_instance;
    static QMutex s_mutex;

    QSqlDatabase m_db;

    void createTables(); //建表
};