#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "core/data/AlarmInfo.h"
#include <memory>

class AlarmPanel : public QWidget
{
    Q_OBJECT
public:
    explicit AlarmPanel(QWidget *parent = nullptr);

    void addAlarm (const AlarmInfo &info); // 添加告警
    void loadFromDb(int taskId = -1); // 从数据库加载告警记录并刷新表格显示
    void clearAlarms(); // 清除所有告警

private slots:
    void onAckSelected(); // 用户选中
    void onAckAll();
    void loadAllAlarms();

private:
    void setupUI();

    QTableWidget *m_table {nullptr};
    QPushButton *m_btnAck {nullptr};
    QPushButton *m_btnAckAll {nullptr};
    QPushButton *m_btnClear {nullptr};
    QLabel *m_lblCount {nullptr};

    int m_unacked {0};
    int m_currentTaskId {-1};
    // 原本计划通过一次性写入更新数据库，发现开销太大，保留到后面扩展
    std::unique_ptr<QList<AlarmInfo>> m_alarmAll;
};