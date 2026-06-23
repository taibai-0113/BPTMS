#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QSqlDatabase>
#include "core/data/TestRecord.h"
#include "core/data/BatteryData.h"

class DbWorker : public QObject {
    Q_OBJECT
public:
    // 传入数据库文件的绝对路径
    explicit DbWorker(const QString &dbPath, QObject *parent = nullptr);
    ~DbWorker();

public slots:
    // 线程启动时的初始化槽
    void onInitDatabase();
    // 异步查询所有任务的槽
    void onQueryAllTasks();
    // 异步插入采样数据的槽
    void onSaveSampleData(BatteryData data, int taskId);

signals:
    // 数据库初始化结果反馈
    void databaseInitialized(bool success);
    // 查询结果数据回传信号（携带查询到的任务列表）
    void tasksQueried(const QList<TestRecord> &tasks);
    // 错误信息捕获信号
    void errorOccurred(const QString &errorMsg);
    // 通知主线程写入成功
    void sampleDataSaved();

private:
    void createTables(QSqlDatabase &db);

    QString m_dbPath;
    QString m_connName; // 每个线程独立的数据库连接名称
};