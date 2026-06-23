#include "DatabaseManager.h"
#include "common/logger/Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>

DatabaseManager *DatabaseManager::s_instance = nullptr;
QMutex DatabaseManager::s_mutex;

DatabaseManager *DatabaseManager::instance()
{
    QMutexLocker lock(&s_mutex);
    if(!s_instance) s_instance = new DatabaseManager();
    return s_instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent){}

bool DatabaseManager::init(const QString &path)
{
    // path 是数据库文件的完整路径（例如 "C:/data/app.db"）。
    // QFileInfo(path).absolutePath() 提取出文件所在的目录部分（例如 "C:/data"）。
    // QDir().mkpath(...) 确保该目录存在，如果不存在则递归创建。

    // 创建目录
    QDir().mkpath(QFileInfo(path).absolutePath());

    // 建里链接
    m_db = QSqlDatabase::addDatabase("QSQLITE", "bptms_conn");
    // 打开数据库
    m_db.setDatabaseName(path);

    if(!m_db.open())
    {
        LOG_E(QStringLiteral("数据库打开失败：%1").arg(m_db.lastError().text()));
        return false;
    }

    // 开启 WAL 模式提高写入性能
    QSqlQuery q(m_db);
    q.exec("PRAGMA journal_mode=WAL;");
    q.exec("PRAGMA synchronous=NORMAL;");
    q.exec("PRAGMA foreign_keys=ON;");

    createTables();
    LOG_I(QStringLiteral("数据库初始化成功：%1").arg(path));
    return true;
}

bool DatabaseManager::isOpen() const {return m_db.isOpen();}
QSqlDatabase DatabaseManager::database() const {return m_db;}

bool DatabaseManager::execSQL(const QString &sql)
{
    QSqlQuery q(m_db);
    // 执行sql语句
    if(!q.exec(sql))
    {
        LOG_E(QStringLiteral("SQL 执行失败：%1\n SQL：%2")
                  .arg(q.lastError().text(),sql));
        return false;
    }
    return true;
}

void DatabaseManager::createTables()
{
    // tasks 表
    execSQL(R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT    NOT NULL,
            description TEXT    DEFAULT '',
            status      TEXT    NOT NULL DEFAULT 'created',
            start_time  TEXT,
            end_time    TEXT,
            sample_count INTEGER DEFAULT 0
        )
    )");

    // sample_data 表
    execSQL(R"(
        CREATE TABLE IF NOT EXISTS sample_data (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            task_id       INTEGER NOT NULL,
            timestamp     TEXT    NOT NULL,
            total_voltage REAL,
            total_current REAL,
            soc           INTEGER,
            soh           INTEGER,
            temp1         REAL,
            temp2         REAL,
            temp3         REAL,
            cell_v0       REAL, cell_v1 REAL, cell_v2 REAL, cell_v3 REAL,
            cell_v4       REAL, cell_v5 REAL, cell_v6 REAL, cell_v7 REAL,
            status_flags  INTEGER,
            cycle_count   INTEGER,
            FOREIGN KEY(task_id) REFERENCES tasks(id)
        )
    )");
    execSQL("CREATE INDEX IF NOT EXISTS idx_sample_task ON sample_data(task_id)");
    execSQL("CREATE INDEX IF NOT EXISTS idx_sample_ts   ON sample_data(timestamp)");

    // alarm_log 表
    execSQL(R"(
        CREATE TABLE IF NOT EXISTS alarm_log (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            task_id      INTEGER DEFAULT -1,
            type         TEXT    NOT NULL,
            actual_value REAL,
            threshold    REAL,
            timestamp    TEXT    NOT NULL,
            acknowledged INTEGER DEFAULT 0
        )
    )");
    execSQL("CREATE INDEX IF NOT EXISTS idx_alarm_task ON alarm_log(task_id)");
}