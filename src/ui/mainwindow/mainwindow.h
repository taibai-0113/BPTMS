#pragma once

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QTimer>
#include "core/data/BatteryData.h"
#include "core/data/AlarmInfo.h"
#include "core/data/TestRecord.h"
#include "communication/DbWorker.h"

class CommWorker;
class AlarmChecker;
class DevicePanel;
class MonitorPanel;
class TestPanel;
class HistoryPanel;
class AlarmPanel;
class LogPanel;
class DatabaseManager;
class SampleDataDao;
class AlarmLogDao;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow()override;

protected:
    // 关闭BPTMS
    void closeEvent(QCloseEvent *event)override;

private slots:
    // 关于所有操作的槽函数
    void onBatteryDataReady(BatteryData data);
    void onAlarmTriggered  (AlarmInfo   info);
    void onConnectionChanged(bool connected);
    void onCommError(QString error);
    void onFrameStats(int total, int failed);
    void updateClock();

    // 工具栏/菜单槽
    void onConnectClicked();
    void onConnectRequested (bool sim, const QString &port, int baud);
    void onDisconnectRequested();
    void onStartTask(TestRecord taskId);
    void onPauseTask();
    void onStopTask();
    void transferStartTask();
    void showCreateTaskDialog();
    void showAlarmConfigDialog();
    void showDeviceConfigDialog();
    void showAbout();

signals:
    // 需要更新的作用操作，通过信号发出
    void sigConnectDevice(bool connect);
    void sigDisconnectDevice();
    void sigSetInterval(int interval);
    void requestSaveSampleData(BatteryData data, int taskId);

private:
    // void setupUI();
    // 一些关于界面的UI函数
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupDockWidget();
    void setupCentralWidget();
    void setupWorkerThread();
    void connectSignals();
    void applyStyleSheet();

    // ── Worker 线程 ───────────────────────────────────
    QThread *m_workerThread {nullptr};
    CommWorker *m_commWorker {nullptr};

    // ── 数据库 线程 ───────────────────────────────────
    QThread *dbThread {nullptr};
    DbWorker *dbWorker {nullptr};

    // ── 业务组件 ───────────────────────────────────
    AlarmChecker *m_alarmChecker {nullptr};
    int m_currentTaskId {-1};

    // ── UI 组件 ───────────────────────────────────────
    DevicePanel  *m_devicePanel  { nullptr };
    MonitorPanel *m_monitorPanel { nullptr };
    TestPanel    *m_testPanel    { nullptr };
    HistoryPanel *m_historyPanel { nullptr };
    AlarmPanel   *m_alarmPanel   { nullptr };
    LogPanel     *m_logPanel     { nullptr };

    // ── 状态栏标签 ────────────────────────────────────
    QLabel *m_lblConnStatus { nullptr };
    QLabel *m_lblSampleRate { nullptr };
    QLabel *m_lblDbCount    { nullptr };
    QLabel *m_lblAlarmCount { nullptr };
    QLabel *m_lblClock      { nullptr };
    QTimer *m_clockTimer    { nullptr };

    int m_unackedAlarms { 0 };
    int m_dbRowCount    { 0 };
    int m_sampleCount    { 0 };
    bool m_paused {false};
    inline static TestRecord currentTask{};
};