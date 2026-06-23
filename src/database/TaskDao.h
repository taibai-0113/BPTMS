#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "core/data/TestRecord.h"

// 换汤不换药
class TaskDao : public QObject
{
    Q_OBJECT
public:
    explicit TaskDao(const QSqlDatabase &db, QObject *parent = nullptr);

    int insert (const TestRecord &rec);
    bool updateStatus (const TestRecord &rec);
    int getMaxId() const;
    TestRecord queryById (int id) const;
    QList<TestRecord> queryAll () const;
    bool remove (int id);

private:
    QSqlDatabase m_db;
    static TestRecord rowToRecord(const QSqlQuery &q);
};