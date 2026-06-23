#include "SampleDataDao.h"
#include "common/logger/Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QTextStream>

SampleDataDao::SampleDataDao(const QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_db(db) {}

bool SampleDataDao::insert(int taskId, const BatteryData &d)
{
    QSqlQuery q(m_db);
    q.prepare(R"(
        INSERT INTO sample_data
            (task_id, timestamp, total_voltage, total_current,
             soc, soh, temp1, temp2, temp3,
             cell_v0, cell_v1, cell_v2, cell_v3,
             cell_v4, cell_v5, cell_v6, cell_v7,
             status_flags, cycle_count)
        VALUES
            (:tid, :ts, :tv, :tc,
             :soc, :soh, :t1, :t2, :t3,
             :c0, :c1, :c2, :c3,
             :c4, :c5, :c6, :c7,
             :sf, :cc)
    )");

    q.bindValue(":tid", taskId);
    q.bindValue(":ts", d.timestamp.toString(Qt::ISODateWithMs));
    q.bindValue(":tv", d.totalVoltage);
    q.bindValue(":tc", d.totalCurrent);
    q.bindValue(":soc", d.soc);
    q.bindValue(":soh", d.soh);
    q.bindValue(":t1", d.temp1);
    q.bindValue(":t2", d.temp2);
    q.bindValue(":t3", d.temp3);
    for(int i = 0; i < 8; ++i)q.bindValue(QString(":c%1").arg(i), d.cellVoltages[i]);
    q.bindValue(":sf", d.statusFlags);
    q.bindValue(":cc", d.cycleCount);

    if(!q.exec())
    {
        LOG_E(QStringLiteral("insert sample_data 失败：%1").arg(q.lastError().text()));
        return false;
    }
    return true;
}

bool SampleDataDao::insertBatch(int taskId, const QList<BatteryData> &list)
{
    m_db.transaction(); // 提交数据库插入事务
    for (const auto &d : list)
    {
        // 如果插入失败回滚事务，并撤销之前已插入的所有数据、返回失败
        if(!insert(taskId, d)){m_db.rollback(); return false;}
    }
    return m_db.commit(); // 插入成功
}

QList<BatteryData> SampleDataDao::queryByTaskId(
    int taskId, const QDateTime &from, const QDateTime &to) const
{
    QSqlQuery q(m_db);
    QString sql = "SELECT * FROM sample_data WHERE task_id=:tid";
    if (from.isValid()) sql += " AND timestamp >= :from";
    if (to.isValid())   sql += " AND timestamp <= :to";
    sql += " ORDER BY id ASC";

    q.prepare(sql);
    q.bindValue(":tid", taskId);
    if (from.isValid()) q.bindValue(":from", from.toString(Qt::ISODateWithMs));
    if (to.isValid())   q.bindValue(":to",   to.toString(Qt::ISODateWithMs));

    QList<BatteryData> list;
    if (q.exec())
        while (q.next()) list.append(rowToData(q));
    return list;
}

int SampleDataDao::countByTaskId(int taskId)const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM sample_data WHERE task_id=:tid");
    q.bindValue(":tid", taskId);
    return (q.exec() && q.next() ? q.value(0).toInt() : 0);
}

bool SampleDataDao::deleteByTaskId(int taskId)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM sample_data WHERE task_id=:tid");
    q.bindValue(":tid", taskId);
    return q.exec();
}

QString SampleDataDao::exportCsv(int taskId)const
{
    auto list = queryByTaskId(taskId);
    QString out; // 中转数据流
    QTextStream ts(&out);
    ts << "timestamp,voltage,current,soc,soh,temp1,temp2,temp3,"
          "cell_v0,cell_v1,cell_v2,cell_v3,cell_v4,cell_v5,cell_v6,cell_v7,"
          "status_flags,cycle_count\n";
    for(const auto &d : list)
    {
        ts << d.timestamp.toString(Qt::ISODateWithMs) << ','
           << d.totalVoltage << ',' << d.totalCurrent << ','
           << d.soc << ',' << d.soh << ','
           << d.temp1 << ',' << d.temp2 << ',' << d.temp3;
        for(double cv : d.cellVoltages)ts << ',' << cv;
        ts << ',' << d.statusFlags << ',' << d.cycleCount << '\n';
    }
    return out;
}

BatteryData SampleDataDao::rowToData(const QSqlQuery &q)
{
    BatteryData d;
    d.timestamp = QDateTime::fromString(q.value("timestamp").toString(), Qt::ISODateWithMs);
    d.totalVoltage = q.value("total_voltage").toDouble();
    d.totalCurrent = q.value("total_current").toDouble();
    d.soc = q.value("soc").toInt();
    d.soh = q.value("soh").toInt();
    d.temp1 = q.value("temp1").toDouble();
    d.temp2 = q.value("temp2").toDouble();
    d.temp3 = q.value("temp3").toDouble();
    for(int i = 0; i < 8; ++i)
        d.cellVoltages[i] = q.value(QStringLiteral("cell_v%1").arg(i)).toDouble();
    d.statusFlags = static_cast<uint16_t>(q.value("status_flags").toUInt());
    d.cycleCount = q.value("cycle_count").toInt();
    return d;
}