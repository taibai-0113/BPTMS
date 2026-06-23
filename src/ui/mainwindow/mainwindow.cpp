#include "MainWindow.h"
#include "communication/CommWorker.h"
#include "core/AlarmChecker.h"
#include "core/data/TestRecord.h"
#include "common/config/AppConfig.h"
#include "common/logger/Logger.h"
#include "database/DatabaseManager.h"
#include "database/SampleDataDao.h"
#include "database/AlarmLogDao.h"
#include "database/TaskDao.h"
#include "ui/panels/DevicePanel.h"
#include "ui/panels/MonitorPanel.h"
#include "ui/panels/TestPanel.h"
#include "ui/panels/HistoryPanel.h"
#include "ui/panels/AlarmPanel.h"
#include "ui/panels/LogPanel.h"
#include "ui/dialogs/CreateTaskDialog.h"
#include "ui/dialogs/AlarmConfigDialog.h"
#include "ui/dialogs/DeviceConfigDialog.h"

#include <QDockWidget>
#include <QTabWidget>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFile>
#include <QApplication>
#include <QAction>
#include <QDateTime>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("BPTMS · 锂电池组测试与管理系统 v1.0"));
    this->setWindowIcon(QIcon(":/icon/mainIcon.svg")); // 放置icon
    // 1. 设置最小尺寸，防止用户把窗口缩得太小导致 UI 挤压错位
    setMinimumSize(800, 450);

    // 2. 设置“常规状态”下的默认尺寸。
    // 这样当用户取消最大化（向下还原）时，窗口会变成 1280x800
    resize(1280, 720);

    // 3. 核心：直接告诉操作系统，启动时将窗口最大化
    setWindowState(Qt::WindowMaximized);

    applyStyleSheet(); // QSS
    m_testPanel = new TestPanel(this); // 测试任务面板，放到这里是因为顶部工具栏跟菜单栏需要用到
    m_devicePanel = new DevicePanel(this); // 设备面板，放到这里是因为顶部工具栏跟菜单栏需要用到
    setupMenuBar(); // 菜单栏
    setupToolBar(); // 工具栏
    setupCentralWidget(); // QTabWidget 初始化
    setupDockWidget(); // 两个停靠面板
    setupStatusBar(); // 底部状态栏

    // 主线程优先完成数据库建表
    DatabaseManager::instance()->init("app.db");

    setupWorkerThread(); // CommWorker线程
    connectSignals(); // 信号槽连接

    // 确保数据库连接完全建立后，再启动线程进入事件循环
    m_workerThread->start();
    dbThread->start();

    // 完成任务面板刷新
    m_testPanel->refreshTaskList();

    LOG_I("MainWindow 初始化完成");
}

MainWindow::~MainWindow()
{
    if (m_workerThread && m_workerThread->isRunning()) {
        QMetaObject::invokeMethod(m_commWorker, "disconnectDevice", Qt::QueuedConnection);
        m_workerThread->quit();// 终制线程
        m_workerThread->wait(3000);// 等待3秒
    }
    if (dbThread && dbThread->isRunning()) {
        dbThread->quit();
        dbThread->wait(3000);
    }
}

// ─── closeEvent ──────────────────────────────────────────
// 绑定QCloseEvent *event代表该函数接管了关闭窗口行为的决策
void MainWindow::closeEvent(QCloseEvent *event)
{
    // 关闭窗口前先保存数据
    AppConfig::instance()->saveIni();
    AppConfig::instance()->saveAlarmConfig();

    auto ret = QMessageBox::question(
        this, tr("退出确认"),
        tr("确定要退出 BPTMS 吗？\n当前连接和测试任务将被终止。"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if(ret == QMessageBox::Yes)
    {
        LOG_I("用户主动退出程序");
        event->accept(); // 关闭该窗口
    }else{
        event->ignore(); // 取消关闭窗口
    }
}

// ─── UI 搭建 ─────────────────────────────────────────────
void MainWindow::setupMenuBar()
{
    // 文件菜单
    QMenu *menuFile = menuBar()->addMenu(tr("文件(&F)"));
    menuFile->addAction(tr("退出(&Q)"), this, &QWidget::close, QKeySequence::Quit);

    // 设备
    QMenu *menuDevice = menuBar()->addMenu(tr("设备(&D)"));
    menuDevice->addAction(tr("设备配置..."), this, &MainWindow::showDeviceConfigDialog);
    menuDevice->addSeparator();
    QAction *actConnect = menuDevice->addAction(tr("连接设备"));
    connect(actConnect, &QAction::triggered, this, [this](){
        m_devicePanel->onConnectClicked();
    });
    menuDevice->addAction(tr("断开设备"), this, &MainWindow::onDisconnectRequested);

    // 任务菜单 m_testPanel->currentTaskId()
    QMenu *menuTask = menuBar()->addMenu(tr("任务(&T)"));
    menuTask->addAction(tr("启动(&S)"), m_testPanel, &TestPanel::currentTaskId, QKeySequence("Ctrl + R"));
    menuTask->addAction(tr("暂停/继续(&P)"), this, &MainWindow::onPauseTask, QKeySequence("Ctrl + P"));
    menuTask->addAction(tr("停止(&E)"), this, &MainWindow::onStopTask, QKeySequence("Ctrl + E"));

    // 配置菜单
    QMenu *menuConfig = menuBar()->addMenu(tr("配置(&C)"));
    menuConfig->addAction(tr("告警阈值..."), this, &MainWindow::showAlarmConfigDialog);

    // 帮助菜单
    QMenu *menuHelp = menuBar()->addMenu(tr("帮助(&H)"));
    menuHelp->addAction(tr("关于 BPTMS..."), this, &MainWindow::showAbout);
}

void MainWindow::setupToolBar()
{
    QToolBar *tb = addToolBar(tr("主工具栏"));
    tb->setMovable(false);
    tb->setIconSize(QSize(24, 24));
    // icon在左边，text在右边
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto makeBtn = [&](const QString &iconPath, const QString &text, std::function<void()> slot) -> QAction*
    {
        QAction *a = nullptr;
        if (!iconPath.isEmpty())
            a = tb->addAction(QIcon(iconPath), text);
        else
            a = tb->addAction(text);

        // 直接连接到 std::function
        connect(a, &QAction::triggered, this, slot);
        return a;
    };

    makeBtn(":/icon/connect.svg", tr("连接"), [this](){ m_devicePanel->onConnectClicked(); });
    makeBtn(":/icon/disConnect.svg", tr("断开"), [this](){ onDisconnectRequested(); });

    tb->addSeparator();

    makeBtn(":/icon/startTask.svg", tr("开始任务"), [this](){ transferStartTask(); });
    makeBtn(":/icon/pauseTask.svg", tr("暂停"), [this](){ onPauseTask(); });
    makeBtn(":/icon/stopTask.svg", tr("停止"), [this](){ onStopTask(); });

    tb->addSeparator();

    makeBtn(":/icon/serialPortSettings.svg", tr("设备配置"),  [this](){ showDeviceConfigDialog(); });
    makeBtn(":/icon/alarmConfig.svg", tr("告警配置"),  [this](){ showAlarmConfigDialog(); });
}

void MainWindow::setupCentralWidget()
{
    QTabWidget *tab = new QTabWidget(this);
    tab->setDocumentMode(true);

    m_monitorPanel = new MonitorPanel(this); // 主面板
    m_historyPanel = new HistoryPanel(this); // 历史数据面板
    m_alarmPanel   = new AlarmPanel(this); // 告警面板
    m_logPanel     = new LogPanel(this); // 日志面板

    tab->addTab(m_monitorPanel, tr("实时监控"));
    int testTabIndex = tab->addTab(m_testPanel, tr("测试任务"));
    int historyTabIndex = tab->addTab(m_historyPanel, tr("历史数据"));
    tab->addTab(m_alarmPanel,   tr("告警记录"));
    tab->addTab(m_logPanel,   tr("系统日志"));

    // 连接 Tab 切换信号
    connect(tab, &QTabWidget::currentChanged, this, [=](int index)
            {
                if (index == testTabIndex)
                {
                    // 当切换到该 Tab 时，主动调用刷新
                    m_testPanel->refreshTaskList();
                }
                else if (index == historyTabIndex)
                {
                    m_historyPanel->refreshTaskList();
                }
            });

    setCentralWidget(tab);
}

void MainWindow::setupDockWidget()
{
    // ── 左侧：设备面板 Dock ──────────────────────────
    QDockWidget *dockDevice = new QDockWidget(tr("设备控制"), this);
    dockDevice->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea); // 面板可以停靠做左边跟右边
    // 让这个停靠窗口可以移动位置、可以浮出来变成独立窗口，但不能关闭
    dockDevice->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dockDevice->setWidget(m_devicePanel);
    dockDevice->setMinimumWidth(200);
    dockDevice->setMinimumHeight(260);
    addDockWidget(Qt::LeftDockWidgetArea, dockDevice); // 一开始停靠在左边
}

void MainWindow::setupStatusBar()
{
    m_lblConnStatus = new QLabel(tr("  ● 未连接  "), this);
    m_lblSampleRate = new QLabel(tr("  采样: —  "), this);
    m_lblDbCount    = new QLabel(tr("  DB: 0  "),   this);
    m_lblAlarmCount = new QLabel(tr("  告警: 0  "),  this);
    m_lblClock      = new QLabel(this);

    for(auto *lbl : {m_lblConnStatus, m_lblSampleRate,
                      m_lblDbCount, m_lblAlarmCount, m_lblClock})
        lbl->setStyleSheet("color:#aaa; margin:0 4px;");

    statusBar()->addWidget(m_lblConnStatus);
    statusBar()->addWidget(m_lblSampleRate);
    statusBar()->addWidget(m_lblDbCount);
    statusBar()->addWidget(m_lblAlarmCount);
    statusBar()->addWidget(m_lblClock);

    // 时钟
    m_clockTimer = new QTimer(this);
    m_clockTimer->setInterval(1000);
    connect(m_clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    m_clockTimer->start();
    updateClock();
}

void MainWindow::setupWorkerThread()
{
    // 先启动线程，让后将CommWorker放到线程中（注意，此时线程是挂着的，m_commWorker还没工作）
    m_workerThread = new QThread(this);
    m_commWorker = new CommWorker();
    m_commWorker->moveToThread(m_workerThread);

    // 数据库异步写线程
    dbThread = new QThread(this);
    dbWorker = new DbWorker("app.db");
    dbWorker->moveToThread(dbThread);

    m_alarmChecker = new AlarmChecker(this);
    m_alarmChecker->setConfig(AppConfig::instance()->alarmConfig());

    // 绑定线程结束信号，自动清理 worker 对象
    connect(m_workerThread, &QThread::finished, m_commWorker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
    connect(dbThread, &QThread::finished, dbWorker, &QObject::deleteLater);
    connect(dbThread, &QThread::finished, dbThread, &QObject::deleteLater);

    // m_workerThread->start();
    // dbThread->start();

    LOG_I("Worker 线程和 DB 线程已启动");
}

void MainWindow::connectSignals()
{
    // CommWorker -> MainWindow （跨线程，自动QueuedConnection）
    connect(m_commWorker, &CommWorker::batteryDataReady,
            this, &MainWindow::onBatteryDataReady);
    connect(m_commWorker, &CommWorker::errorOccurred,
            this, &MainWindow::onCommError);
    connect(m_commWorker, &CommWorker::connectionStatusChanged,
            this, &MainWindow::onConnectionChanged);
    connect(m_commWorker, &CommWorker::frameStats,
            this, &MainWindow::onFrameStats);

    // TestPanel <-> CommWorker （跨线程）
    connect(m_testPanel, &TestPanel::requestTaskList, dbWorker, &DbWorker::onQueryAllTasks);
    connect(dbWorker, &DbWorker::tasksQueried, m_testPanel, &TestPanel::onTasksReceived);

    // MainWindow -> dbWorker （跨线程）
    connect(dbThread, &QThread::started, dbWorker, &DbWorker::onInitDatabase);
    connect(this, &MainWindow::requestSaveSampleData, dbWorker, &DbWorker::onSaveSampleData);
    connect(dbWorker, &DbWorker::sampleDataSaved, this, [this]{
        ++m_dbRowCount;
        m_lblDbCount->setText(QString("  DB: %1  ").arg(m_dbRowCount));
    });

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, dbThread, &QThread::quit);
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_workerThread, &QThread::quit);

    // MainWindow -> CommWorker （跨线程）
    connect(this, &MainWindow::sigConnectDevice,
            m_commWorker, &CommWorker::connectDevice);
    connect(this, &MainWindow::sigDisconnectDevice,
            m_commWorker, &CommWorker::disconnectDevice);
    connect(this, &MainWindow::sigSetInterval,
            m_commWorker, &CommWorker::setInterval);

    // DevicePanel -> MainWindow
    connect(m_devicePanel, &DevicePanel::connectRequested,
            this, &MainWindow::onConnectRequested);
    connect(m_devicePanel, &DevicePanel::disconnectRequested,
            this, &MainWindow::onDisconnectRequested);
    connect(m_devicePanel, &DevicePanel::intervalChanged,
            this, [this](int ms){
                emit sigSetInterval(ms);
                AppConfig::instance()->setSampleInterval(ms);
            });

    // MonitorPanel -> MainWindow
    connect(this, &MainWindow::sigSetInterval,
            m_monitorPanel, &MonitorPanel::setTimeWindow);

    // AlarmChecker
    connect(m_alarmChecker, &AlarmChecker::alarmTriggered,
            this, &MainWindow::onAlarmTriggered);

    // 未构建
    // Logger → LogPanel
    // 第五个参数：槽函数在哪个线程执行，是立即执行（相同线程）还是排队等一会儿（不同线程）
    connect(Logger::instance(), &Logger::logAppended,
            m_logPanel, &LogPanel::appendLog,
            Qt::QueuedConnection);

    // TestPanel
    connect(m_testPanel, &TestPanel::createTaskRequested,
            this, &MainWindow::showCreateTaskDialog);
    connect(m_testPanel, &TestPanel::startTaskRequested,
            this, &MainWindow::onStartTask);
    connect(m_testPanel, &TestPanel::stopTaskRequested,
            this, &MainWindow::onStopTask);
    connect(m_testPanel, &TestPanel::pauseTaskRequested,
            this, &MainWindow::onPauseTask);

    // HistoryPanel
    connect(m_historyPanel, &HistoryPanel::queryRequested,
            this, [this](int taskId){
                SampleDataDao dao(DatabaseManager::instance()->database(), this);
                auto records = dao.queryByTaskId(taskId);
                // 调用数据
                m_historyPanel->displayData(records);
            });
}

// 加载QSS
void MainWindow::applyStyleSheet()
{
    QFile f(":/styles/dark.qss");
    if(!f.open(QIODevice::ReadOnly))
    {
        // 回退：从文件系统加载
        f.setFileName("resources/styles/dark.qss");
        if(!f.open(QIODevice::ReadOnly))return;
    }
    qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
}

// ─── 槽函数实现 ───────────────────────────────────────────
// 最重要的函数 ---> 展示数据
void MainWindow::onBatteryDataReady(BatteryData data)
{
    // 1. 刷新实时监控面板
    m_monitorPanel->updateData(data);

    // 2. 告警检测（如果有告警就发送信号到onAlarmTriggered）
    m_alarmChecker->checkData(data, m_currentTaskId);

    // 3. 写入数据库：增加严格的状态拦截机制
    bool isTaskActive = (m_currentTaskId >= 0) &&
                        (!m_paused) &&
                        (currentTask.status == TestStatus::Running);

    // 4. 写入数据库（仅在任务运行时）
    if (isTaskActive) {
        // 不在主线程 new DAO，直接将数据发射到后台线程队列
        emit requestSaveSampleData(data, m_currentTaskId);
        ++ m_sampleCount;
    }
}

// 如果有告警就执行该函数
void MainWindow::onAlarmTriggered(AlarmInfo info)
{
    // 写入数据库
    AlarmLogDao dao(DatabaseManager::instance()->database(), this);
    dao.insert(info);

    // 更新告警面板
    m_alarmPanel->addAlarm(info);

    // 更新状态栏 增加告警数量
    ++m_unackedAlarms;
    m_lblAlarmCount->setText(
        QStringLiteral("  ⚠ 告警总数: %1  ").arg(m_unackedAlarms));
    m_lblAlarmCount->setStyleSheet("color:#e74c3c; font-weight:bold; margin:0 4px;");
}

// 更新连接状态
void MainWindow::onConnectionChanged(bool connected)
{
    m_devicePanel->setConnected(
        connected,
        connected ? AppConfig::instance()->simulatorMode()
                        ? tr("模拟器") : AppConfig::instance()->serialPort()
                  : QString());
    if (connected) {
        m_lblConnStatus->setText(tr("  ● 已连接  "));
        m_lblConnStatus->setStyleSheet("color: #00ff00; font-weight:bold; margin:0 4px;");
    } else {
        m_lblConnStatus->setText(tr("  ● 未连接  "));
        m_lblConnStatus->setStyleSheet("color:#e74c3c; margin:0 4px;");
    }
    LOG_I(QStringLiteral("连接状态变更: %1").arg(connected ? "已连接" : "已断开"));
}

// 更新报错状态
void MainWindow::onCommError(QString error)
{
    LOG_E(error);
    statusBar()->showMessage(QStringLiteral("通信错误：%1").arg(error), 5000);
}

// 计算并更新通信质量
void MainWindow::onFrameStats(int total, int failed)
{
    // 这里虽然调用的是槽函数，但是槽函数本身就是普通的成员函数，不用信号槽也可以调用，所以可以实现
    m_devicePanel->updateFrameStats(total, failed);
    //更新底部状态栏
    double rate = total > 0 ? (1.0 - (double)failed / total) * 100.0 : 0.0;
    m_lblSampleRate->setText(
        QStringLiteral("  帧: %1 / 质量: %2%  ").arg(total).arg(rate, 0, 'f', 1));
}

// 定时器更新时间
void MainWindow::updateClock()
{
    m_lblClock->setText(
        QDateTime::currentDateTime().toString("  yyyy-MM-dd hh:mm:ss  "));
}

// 返回用户的选择
void MainWindow::onConnectClicked()
{
    m_alarmPanel->clearAlarms(); // 清理告警面板
    m_monitorPanel->clearDisplay(); // 清理数据面板
    onConnectRequested(
        AppConfig::instance()->simulatorMode(),
        AppConfig::instance()->serialPort(),
        AppConfig::instance()->baudRate());
}

// 根据用户的选择判断用模拟器还是真实串口
void MainWindow::onConnectRequested(bool sim, const QString &port, int baud)
{
    AppConfig::instance()->setSimulatorMode(sim);
    if(!sim)
    {
        AppConfig::instance()->setSerialPort(port);
        AppConfig::instance()->setBaudRate(baud);
    }
    // 向 m_commWorker 发送一个异步请求，请它自己在自己的线程里执行 connectDevice(sim)，
    // 而当前线程不必等待它执行完，可以立即继续运行。
    QMetaObject::invokeMethod(m_commWorker, "connectDevice",
                              Qt::QueuedConnection, Q_ARG(bool, sim));
}

// 调用设备断开的请求
void MainWindow::onDisconnectRequested()
{
    QMetaObject::invokeMethod(m_commWorker, "disconnectDevice",
                              Qt::QueuedConnection);
}

// 创建任务对话框
void MainWindow::showCreateTaskDialog()
{
    CreateTaskDialog dlg(this);
    // 确保用户点击了保存
    if(dlg.exec() == QDialog::Accepted)
    {
        TestRecord  task = dlg.createTask();
        TaskDao dao(DatabaseManager::instance()->database());
        task.id = dao.getMaxId() + 1;
        task.status = TestStatus::Created;
        // m_currentTaskId = task.id;
        dao.insert(task);
        LOG_I("任务添加成功");
        statusBar()->showMessage(tr("任务添加成功"), 3000);
        m_paused = false;
        m_testPanel->setCreateTask();
    }
}

// 启动任务
void MainWindow::onStartTask(TestRecord task)
{
    // 防御性检查：确保当前没有任务在运行
    if (m_currentTaskId >= 0) {
        QMessageBox::warning(this, "提示", "已有任务正在运行");
        return;
    }

    onDisconnectRequested();
    m_alarmPanel->clearAlarms(); // 清理告警面板
    m_monitorPanel->clearDisplay(); // 清理数据面板

    m_sampleCount = 0;
    m_paused = false;
    m_currentTaskId = task.id;
    currentTask = std::move(task);

    currentTask.startTime = QDateTime::currentDateTime();
    currentTask.status = TestStatus::Running;

    TaskDao dao(DatabaseManager::instance()->database());
    dao.updateStatus(currentTask);

    m_testPanel->setCurrentTask(m_currentTaskId, currentTask.name, currentTask.description);
    LOG_I(QStringLiteral("测试任务已启动 ID=%1 Name=%2")
              .arg(m_currentTaskId).arg(task.name));
    statusBar()->showMessage(
        QStringLiteral("任务 [%1] 已开始").arg(currentTask.name), 3000);
    m_alarmPanel->loadFromDb(m_currentTaskId);
    m_devicePanel->onConnectClicked();
}

// 暂停任务
void MainWindow::onPauseTask()
{
    if(m_currentTaskId < 0)
    {
        QMessageBox::warning(this,
                                 tr("告警"),  // 标题
                                 tr("请线启动任务"));
        LOG_I(QStringLiteral("用户未启动任务"));
        return;
    };

    m_paused = !m_paused;
    currentTask.status = m_paused ? TestStatus::Paused : TestStatus::Running;

    TaskDao dao(DatabaseManager::instance()->database());
    dao.updateStatus(currentTask);

    if (m_paused) {
        onDisconnectRequested();
    } else {
        m_devicePanel->onConnectClicked();
    }

    m_testPanel->togglePause();
    LOG_I(QStringLiteral("测试任务 ID=%1 暂停/继续").arg(m_currentTaskId));
}

// 停止任务
void MainWindow::onStopTask()
{
    if(m_currentTaskId < 0)
    {
        QMessageBox::warning(this,
                             tr("告警"),  // 标题
                             tr("请线启动任务"));
        LOG_I(QStringLiteral("用户未启动任务"));
        return;
    };

    onDisconnectRequested();
    currentTask.status = TestStatus::Finished;
    currentTask.endTime = QDateTime::currentDateTime();
    currentTask.sampleCount += m_sampleCount;
    qDebug() << "m_sampleCount" << m_sampleCount;
    qDebug() << "currentTask" << currentTask.sampleCount;

    TaskDao dao(DatabaseManager::instance()->database());
    dao.updateStatus(currentTask);
    // 任务结束，立即重置数据
    currentTask.reset();
    m_sampleCount = 0;
    m_currentTaskId = -1;
    m_paused = false;

    LOG_I(QStringLiteral("测试任务 ID = %1 已停止").arg(m_currentTaskId));
    m_testPanel->clearTask();
    statusBar()->showMessage(tr("任务已停止"), 3000);
}

// 顶部工具栏启动任务中转函数
void MainWindow::transferStartTask()
{
    // 假设当前类中有一个 TestPanel 的指针叫 m_testPanelPtr
    if (m_testPanel) {
        m_testPanel->currentTaskId();
    }
}

// 告警配置对话框
void MainWindow::showAlarmConfigDialog()
{
    AlarmConfigDialog dlg(AppConfig::instance()->alarmConfig(), this);
    // 确保用户点击了保存
    if(dlg.exec() == QDialog::Accepted)
    {
        AlarmConfig cfg = dlg.alarmConfig();
        AppConfig::instance()->setAlarmConfig(cfg);
        m_alarmChecker->setConfig(cfg);
        LOG_I("告警配置已更新");
        statusBar()->showMessage(tr("告警配置已保存"), 3000);
    }
}

// 串口配置对话框
void MainWindow::showDeviceConfigDialog()
{
    DeviceConfigDialog dlg(this);
    LOG_I("设备配置已更新");
    dlg.exec();
}

// 显示BPTMS基本信息
void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("关于 BPTMS"),
                       tr("<h3>BPTMS v1.0</h3>"
                          "<p>锂电池组测试与管理系统</p>"
                          "<p>基于 Qt %1 · C++17</p>"
                          "<p>© 2026 BPTMS Project</p>")
                           .arg(QT_VERSION_STR));
}