#include "TaskDao.h"
#include "common/logger/Logger.h"
#include <QSqlQuery>
#include <QSqlError>

TaskDao::TaskDao(const QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_db(db) {}

int TaskDao::insert(const TestRecord &rec)
{
    QSqlQuery q(m_db);
    q.prepare(R"(
        INSERT INTO tasks (id, name, description, status, start_time, end_time, sample_count)
        VALUES (:id, :name, :desc, :st, :stime, :etime, :scount)
    )");
    q.bindValue(":id",  rec.id);
    q.bindValue(":name",  rec.name);
    q.bindValue(":desc",  rec.description);
    q.bindValue(":st",    TestRecord::statusToString(rec.status));
    q.bindValue(":stime", rec.startTime.toString(Qt::ISODateWithMs));
    q.bindValue(":etime", rec.startTime.toString(Qt::ISODateWithMs));
    q.bindValue(":scount", rec.sampleCount);
    if (!q.exec()) {
        LOG_E(QStringLiteral("insert task 失败: %1").arg(q.lastError().text()));
        return -1;
    }
    return static_cast<int>(q.lastInsertId().toLongLong());
}

bool TaskDao::updateStatus(const TestRecord &rec)
{
    QSqlQuery q(m_db);
    q.prepare(R"(
        UPDATE tasks SET name=:name, description=:desc, status=:st,
        start_time=:stime, end_time=:etime, sample_count=:scount WHERE id=:id
    )"); 
    q.bindValue(":id",  rec.id);
    q.bindValue(":name",  rec.name);
    q.bindValue(":desc",  rec.description);
    q.bindValue(":st",    TestRecord::statusToString(rec.status));
    q.bindValue(":stime", rec.startTime.toString(Qt::ISODateWithMs));
    q.bindValue(":etime", rec.startTime.toString(Qt::ISODateWithMs));
    q.bindValue(":scount", rec.sampleCount);
    return q.exec();
}

TestRecord TaskDao::queryById(int id) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM tasks WHERE id=:id");
    q.bindValue(":id", id);
    if (q.exec() && q.next()) return rowToRecord(q);
    return {};
}

QList<TestRecord> TaskDao::queryAll() const
{
    QSqlQuery q(m_db);
    q.exec("SELECT * FROM tasks ORDER BY id DESC");
    QList<TestRecord> list;
    while (q.next()) list.append(rowToRecord(q));
    return list;
}

int TaskDao::getMaxId() const
{
    QSqlQuery q(m_db);
    q.exec("SELECT MAX(id) FROM tasks");
    if (q.next()) {
        if (q.isNull(0)) {
            return -1;
        }
        return q.value(0).toInt();
    }
    return -1;
}

bool TaskDao::remove(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM tasks WHERE id=:id");
    q.bindValue(":id", id);
    return q.exec();
}

TestRecord TaskDao::rowToRecord(const QSqlQuery &q)
{
    TestRecord r;
    r.id          = q.value("id").toInt();
    r.name        = q.value("name").toString();
    r.description = q.value("description").toString();
    r.status      = TestRecord::statusFromString(q.value("status").toString());
    r.startTime   = QDateTime::fromString(
        q.value("start_time").toString(), Qt::ISODateWithMs);
    r.endTime     = QDateTime::fromString(
        q.value("end_time").toString(), Qt::ISODateWithMs);
    r.sampleCount = q.value("sample_count").toInt();
    return r;
}