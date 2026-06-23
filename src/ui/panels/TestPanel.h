#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QProgressBar>
#include "core/data/TestRecord.h"

class TestPanel : public QWidget
{
    Q_OBJECT
public:
    explicit TestPanel(QWidget *parent = nullptr);

    void setCreateTask();
    void currentTaskId();
    void setCurrentTask(int taskId, const QString &name, const QString &description);
    void clearTask();
    void togglePause();
    void refreshTaskList();

signals:
    void createTaskRequested();
    void startTaskRequested(TestRecord task);
    void pauseTaskRequested();
    void stopTaskRequested();
    void requestTaskList();

public slots:
    void onTasksReceived(const QList<TestRecord> &tasks);


private:
    void setupUI();

    QPushButton *m_btnCreate {nullptr};
    QPushButton *m_btnStart {nullptr};
    QPushButton *m_btnPause {nullptr};
    QPushButton *m_btnStop {nullptr};
    QLabel *m_lblTaskId {nullptr};
    QLabel *m_lblTaskName {nullptr};
    QLabel *m_lblTaskDescription {nullptr};
    QLabel *m_lblStatus {nullptr};
    QLabel *m_lblElapsed {nullptr};
    QTableWidget *m_tblTasks {nullptr};

    QTimer *m_elapsedTimer {nullptr};
    int m_elapsed {0};
    bool m_paused {false};
    int m_currentTaskId {-1};
};