#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "core/data/BatteryData.h"

class SampleDataDao : public QObject
{    Q_OBJECT
public:
    explicit SampleDataDao(const QSqlDatabase &db, QObject *parent = nullptr);

    // 向数据库插入数据
    bool insert(int taskId, const BatteryData &data);
    // 向数据库批量插入数据
    bool insertBatch(int taskId, const QList<BatteryData> &List);

    // 根据Id跟时间段查询数据并返回这个时间段存储的数据
    QList<BatteryData> queryByTaskId(int taskId,
                                     const QDateTime &from = {},
                                     const QDateTime &to ={})const;

    int countByTaskId(int taskId) const; // 根据Id查询数据
    bool deleteByTaskId(int taskId); // 根据Id删除数据

    // 导出为 CSV 字符串
    QString exportCsv(int taskId) const;

private:
    QSqlDatabase m_db;
    // 将SQL查询结果转为BatteryData
    static BatteryData rowToData(const QSqlQuery &q);
};