#include "DbWorker.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>
#include <QSqlQuery>
#include <QDir>
#include <QFileInfo>
#include "database/TaskDao.h"
#include "database/SampleDataDao.h"
#include "common/logger/Logger.h"

DbWorker::DbWorker(const QString &dbPath, QObject *parent)
    : QObject(parent), m_dbPath(dbPath)
{
    m_connName = QString("DbWorker_Conn_%1")
    .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 16);
}

DbWorker::~DbWorker()
{
    QSqlDatabase::removeDatabase(m_connName);
}

void DbWorker::createTables(QSqlDatabase &db)
{
    QSqlQuery q(db);

    // tasks 表
    q.exec(R"(
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
    q.exec(R"(
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
    q.exec("CREATE INDEX IF NOT EXISTS idx_sample_task ON sample_data(task_id)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_sample_ts   ON sample_data(timestamp)");

    // alarm_log 表
    q.exec(R"(
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
    q.exec("CREATE INDEX IF NOT EXISTS idx_alarm_task ON alarm_log(task_id)");
}

void DbWorker::onInitDatabase()
{
    QDir().mkpath(QFileInfo(m_dbPath).absolutePath());

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connName);
    db.setDatabaseName(m_dbPath);

    if (!db.open()) {
        emit errorOccurred(db.lastError().text());
        return;
    }

    QSqlQuery q(db);
    q.exec("PRAGMA journal_mode=WAL;");
    q.exec("PRAGMA synchronous=NORMAL;");
    q.exec("PRAGMA foreign_keys=ON;");

    LOG_I(QStringLiteral("后台数据库初始化成功，线程 ID: 0x%1")
              .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 16));
    emit databaseInitialized(true);
}

void DbWorker::onQueryAllTasks()
{
    QSqlDatabase db = QSqlDatabase::database(m_connName);
    if (!db.isOpen()) {
        emit errorOccurred("Database is not open in worker thread.");
        return;
    }

    TaskDao dao(db);
    QList<TestRecord> tasks = dao.queryAll();
    emit tasksQueried(tasks);
}

void DbWorker::onSaveSampleData(BatteryData data, int taskId)
{
    QSqlDatabase db = QSqlDatabase::database(m_connName);
    if (!db.isOpen()) return;

    SampleDataDao dao(db);
    if (dao.insert(taskId, data)) {
        emit sampleDataSaved();   // 成功后通知主线程更新计数
    }
}