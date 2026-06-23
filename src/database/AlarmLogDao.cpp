#include "AlarmLogDao.h"
#include "common/logger/Logger.h"
#include <QSqlQuery>
#include <QSqlError>

AlarmLogDao::AlarmLogDao(const QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_db(db) {}

bool AlarmLogDao::insert(AlarmInfo &info)
{
    QSqlQuery q(m_db);
    q.prepare(R"(
        INSERT INTO alarm_log (task_id, type, actual_value, threshold, timestamp, acknowledged)
        VALUES (:tid, :type, :av, :thr, :ts, 0)
    )");
    q.bindValue(":tid",  info.taskId);
    q.bindValue(":type", info.typeStr);
    q.bindValue(":av",   info.actualValue);
    q.bindValue(":thr",  info.threshold);
    q.bindValue(":ts",   info.timestamp.toString(Qt::ISODateWithMs));
    if (!q.exec()) {
        LOG_E(QStringLiteral("insert alarm_log 失败: %1").arg(q.lastError().text()));
        return false;
    }
    // 获取自增主键并赋值给 info
    info.id = q.lastInsertId().toInt();
    return true;
}

// 看看数据中是否有符合id且告警已经被解决的数据
// acknowledged = 0 未解决；acknowledged = 1 已解决
bool AlarmLogDao::acknowledge(int id)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE alarm_log SET acknowledged = 1 WHERE id=:id");
    q.bindValue(":id", id);
    if (!q.exec()) { // 错误日志
        LOG_E(QStringLiteral("acknowledge alarm_log 失败: %1").arg(q.lastError().text()));
        return false;
    }
    return true;
}

// LIMIT :lim ---> 返回数据条数
QList<AlarmInfo> AlarmLogDao::queryAll(int limit) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM alarm_log ORDER BY id DESC LIMIT :lim");
    q.bindValue(":lim", limit);
    QList<AlarmInfo> list;
    if (q.exec()) {
        while (q.next()) list.append(rowToInfo(q));
    } else { // 查询失败日志
        LOG_E(QStringLiteral("queryAll alarm_log 失败: %1").arg(q.lastError().text()));
    }
    return list;
}

QList<AlarmInfo> AlarmLogDao::queryByTask(int taskId) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM alarm_log WHERE task_id=:tid ORDER BY id DESC");
    q.bindValue(":tid", taskId);
    QList<AlarmInfo> list;
    if (q.exec()) {
        while (q.next()) list.append(rowToInfo(q));
    } else { // 查询失败日志
        LOG_E(QStringLiteral("queryByTask alarm_log 失败: %1").arg(q.lastError().text()));
    }
    return list;
}

bool AlarmLogDao::updateAckStatus(int alarmId, int acked) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE alarm_log SET acknowledged = ? WHERE id = ?");
    query.addBindValue(acked);
    query.addBindValue(alarmId);
     // 失败日志
    if (!query.exec()) {
        LOG_E(QStringLiteral("updateAckStatus 失败: %1").arg(query.lastError().text()));
        return false;
    }
    return true;
}

// 一键更新所有未确认的告警（用于全部确认）
bool AlarmLogDao::ackAllAlarms(int taskId)
{
    QSqlQuery query(m_db);

    // 动态构建 SQL：如果 taskId >= 0，则附加 task_id 过滤条件
    if (taskId >= 0) {
        query.prepare("UPDATE alarm_log SET acknowledged = 1 WHERE acknowledged = 0 AND task_id = :tid");
        query.bindValue(":tid", taskId);
    } else {
        // taskId < 0 依然保持全局一键确认逻辑
        query.prepare("UPDATE alarm_log SET acknowledged = 1 WHERE acknowledged = 0");
    }
    // 失败日志
    if (!query.exec()) {
        LOG_E(QStringLiteral("ackAllAlarms alarm_log 失败: %1").arg(query.lastError().text()));
        return false;
    }
    return true;
}

// 告警数据中没有被解决的告警
int AlarmLogDao::unackedCount() const
{
    QSqlQuery q(m_db);
    if (!q.exec("SELECT COUNT(*) FROM alarm_log WHERE acknowledged=0")) { // 错误日志
        LOG_E(QStringLiteral("unackedCount alarm_log 失败: %1").arg(q.lastError().text()));
        return 0;
    }
    return (q.next()) ? q.value(0).toInt() : 0;
}

AlarmInfo AlarmLogDao::rowToInfo(const QSqlQuery &q)
{
    AlarmInfo info;
    info.typeStr     = q.value("type").toString();
    info.actualValue = q.value("actual_value").toDouble();
    info.threshold   = q.value("threshold").toDouble();
    info.timestamp   = QDateTime::fromString(
        q.value("timestamp").toString(), Qt::ISODateWithMs);
    info.taskId      = q.value("task_id").toInt();
    info.id          = q.value("id").toInt();
    info.acknowledged= q.value("acknowledged").toBool();
    return info;
}