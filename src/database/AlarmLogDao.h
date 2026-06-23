#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QSqlQuery>
#include "core/data/AlarmInfo.h"

// 跟其他数据库差不多 换汤不换药
class AlarmLogDao : public QObject
{
    Q_OBJECT
public:
    explicit AlarmLogDao(const QSqlDatabase &db, QObject *parent = nullptr);

    bool insert (AlarmInfo &info);
    bool acknowledge(int id);

    QList<AlarmInfo> queryAll (int limit = 200) const;
    QList<AlarmInfo> queryByTask (int taskId) const;

    bool updateAckStatus (int taskId, int acked);
    bool ackAllAlarms (int taskId = -1);
    int unackedCount() const;

private:
    QSqlDatabase m_db;
    static AlarmInfo rowToInfo(const QSqlQuery &q);
};