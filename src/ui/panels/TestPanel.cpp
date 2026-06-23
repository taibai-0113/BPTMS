#include "TestPanel.h"
#include "database/DatabaseManager.h"
#include "database/TaskDao.h"
#include "common/logger/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QDateTime>
#include <QMessageBox>

TestPanel::TestPanel(QWidget *parent) : QWidget(parent)
{
    setupUI();
    // 记录运行时间
    m_elapsedTimer = new QTimer(this);
    m_elapsedTimer->setInterval(1000);
    connect(m_elapsedTimer, &QTimer::timeout, this, [this](){
        if (!m_paused) {
            ++m_elapsed;
            int h = m_elapsed/3600, m=(m_elapsed%3600)/60, s=m_elapsed%60;
            m_lblElapsed->setText(QStringLiteral("%1:%2:%3")
                                      .arg(h,2,10,QLatin1Char('0'))
                                      .arg(m,2,10,QLatin1Char('0'))
                                      .arg(s,2,10,QLatin1Char('0')));
        }
    });
    // UI 组件尚未完成信号绑定时请求数据 需要删除
    // refreshTaskList();
}

void TestPanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8,8,8,8);
    root->setSpacing(8);

    // ── 当前任务信息卡 ───────────────────────────────
    auto *grpCurrent = new QGroupBox(tr("当前任务"),this);
    auto *form = new QFormLayout(grpCurrent);

    m_lblTaskId = new QLabel(tr("-"),this);
    m_lblTaskName = new QLabel(tr("-"),this);
    m_lblTaskDescription = new QLabel(tr("-"),this);
    m_lblStatus = new QLabel(tr("空闲"),this);
    m_lblStatus->setStyleSheet("color:#3498db; font-weight:bold;");
    m_lblElapsed = new QLabel("00:00:00", this);

    m_lblTaskDescription->setMaximumWidth(400);  // 限制最大宽度，超过后换行
    m_lblTaskDescription->setWordWrap(true); // 自动换行
    m_lblTaskDescription->setAlignment(Qt::AlignTop); // 顶部对齐
    m_lblTaskDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


    auto *lblTaskId = new QLabel(tr("任 务 ID："), this);
    auto *lblTaskName = new QLabel(tr("任务名称："), this);
    auto *lblTaskDescription = new QLabel(tr("任务描述："), this);
    auto *lblStatus = new QLabel(tr("状    态："), this);
    auto *lblElapsed = new QLabel(tr("已 运 行："), this);

    form->addRow(lblTaskId, m_lblTaskId);
    form->addRow(lblTaskName, m_lblTaskName);
    form->addRow(lblTaskDescription, m_lblTaskDescription);
    form->addRow(lblStatus, m_lblStatus);
    form->addRow(lblElapsed, m_lblElapsed);
    lblTaskId->setProperty("stat", true);
    lblTaskName->setProperty("stat", true);
    lblTaskDescription->setProperty("stat", true);
    lblStatus->setProperty("stat", true);
    lblElapsed->setProperty("stat", true);
    m_lblTaskId->setProperty("stat", true);
    m_lblTaskName->setProperty("stat", true);
    m_lblTaskDescription->setProperty("stat", true);
    m_lblStatus->setProperty("stat", true);
    m_lblElapsed->setProperty("stat", true);

    root->addWidget(grpCurrent);

    // ── 控制按钮 ─────────────────────────────────────
    auto *hBtn = new QHBoxLayout;
    m_btnCreate = new QPushButton(tr("创建任务"), this);
    m_btnStart = new QPushButton(tr("启动任务"), this);
    m_btnPause = new QPushButton(tr("暂停任务"),    this);
    m_btnStop  = new QPushButton(tr("停止任务"),    this);

    m_btnCreate->setIcon(QIcon(":/icon/createTask.svg"));
    m_btnCreate->setIconSize(QSize(24, 24));
    m_btnStart->setIcon(QIcon(":/icon/startTask.svg"));
    m_btnStart->setIconSize(QSize(24, 24));
    m_btnPause->setIcon(QIcon(":/icon/pauseTask.svg"));
    m_btnPause->setIconSize(QSize(24, 24));
    m_btnStop->setIcon(QIcon(":/icon/stopTask.svg"));
    m_btnStop->setIconSize(QSize(24, 24));

    m_btnStart->setObjectName("btnStart");
    m_btnStop->setObjectName("btnStop");
    m_btnPause->setObjectName("btnPause");
    m_btnPause->setEnabled(false);
    m_btnStop->setEnabled(false);

    hBtn->addWidget(m_btnCreate);
    hBtn->addWidget(m_btnStart);
    hBtn->addWidget(m_btnPause);
    hBtn->addWidget(m_btnStop);
    root->addLayout(hBtn);

    // ── 历史任务列表 ─────────────────────────────────────
    auto *grpHist = new QGroupBox(tr("历史任务"), this);
    auto *layHist = new QVBoxLayout(grpHist);
    m_tblTasks = new QTableWidget(0, 6, this);
    m_tblTasks->setHorizontalHeaderLabels({tr("ID"), tr("名称"),
                                           tr("状态"), tr("开始时间"), tr("结束时间"), tr("样本数")});
    m_tblTasks->verticalHeader()->setVisible(false); // 隐藏垂直表头（左上角的全选按钮）
    m_tblTasks->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tblTasks->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tblTasks->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblTasks->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblTasks->setAlternatingRowColors(true);
    layHist->addWidget(m_tblTasks);
    root->addWidget(grpHist, 1);

    // 信号
    connect(m_btnCreate, &QPushButton::clicked, this, &TestPanel::createTaskRequested);
    connect(m_btnStart, &QPushButton::clicked, this, &TestPanel::currentTaskId);
    connect(m_btnPause, &QPushButton::clicked, this, &TestPanel::pauseTaskRequested);
    connect(m_btnStop, &QPushButton::clicked, this, &TestPanel::stopTaskRequested);
}

// 根据启动规则禁用控件
void TestPanel::setCreateTask()
{
    m_elapsed = 0;
    m_paused = false;
    m_btnStart->setEnabled(true);
    m_btnPause->setEnabled(false);
    m_btnStop->setEnabled(false);
    refreshTaskList();
}

void TestPanel::currentTaskId()
{
    // 表示表格中当前选中的单元格区域列表
    const auto ranges = m_tblTasks->selectedRanges();
    if (ranges.isEmpty())
    {
        QMessageBox::information(this,
                             tr("提示"),  // 标题
                             tr("请从下方任务列表中选择一行来启动任务（若选中多行，将以第一行为准）。"));
        LOG_I(QStringLiteral("未选中任务"));
        return;
    }
    // 没有选中区域则退出
    int row = ranges.first().topRow(); // 获取第一个选中区域的首行
    auto item = m_tblTasks->item(row, 0);
    if (!item) return; // 防御性检查
    int id = item->text().toInt(); // 获取Id

    if(!DatabaseManager::instance()->isOpen()) return;
    TaskDao dao(DatabaseManager::instance()->database()); // 根据Id获取任务信息
    TestRecord task = dao.queryById(id); // 查询所有数据

    LOG_I(QStringLiteral("已获取任务Id：%1，开始启动任务").arg(id));
    emit startTaskRequested(task);
}

// 启动任务
void TestPanel::setCurrentTask(int taskId, const QString &name, const QString &description)
{
    m_currentTaskId = taskId;
    m_elapsed = 0;
    m_paused = false;
    m_lblTaskId->setText(QStringLiteral("#%1").arg(taskId));
    m_lblTaskName->setText(name);
    m_lblTaskDescription->setText(description);
    m_lblStatus->setText("运行中");
    m_lblStatus->setStyleSheet("color:#27ae60; font-weight:bold;");
    m_btnCreate->setEnabled(false);
    m_btnStart->setEnabled(false);
    m_btnPause->setEnabled(true);
    m_btnStop->setEnabled(true);
    m_elapsedTimer->start();
    refreshTaskList();
}

// 清除任务
void TestPanel::clearTask()
{
    m_currentTaskId = -1;
    m_elapsedTimer->stop();
    m_lblTaskId->setText("-");
    m_lblTaskName->setText("-");
    m_lblTaskDescription->setText("-");
    m_lblStatus->setText("空闲");
    m_lblStatus->setStyleSheet("color:#3498db; font-weight:bold;");
    m_lblElapsed->setText("00:00:00");
    m_btnCreate->setEnabled(true);
    m_btnStart->setEnabled(true);
    m_btnPause->setEnabled(false);
    m_btnStop->setEnabled(false);
    refreshTaskList();
}

// 暂停任务
void TestPanel::togglePause()
{
    m_paused = !m_paused;
    m_btnPause->setText(m_paused ? tr("继续") : tr("暂停"));
    m_lblStatus->setText(m_paused ? tr("已暂停") : tr("运行中"));
    m_lblStatus->setStyleSheet(
        m_paused ? "color:#e67e22; font-weight:bold;"
                 : "color:#27ae60; font-weight:bold;");
}

// 刷新任务列表
void TestPanel::refreshTaskList()
{
    // 发送信号激活后台工作线程
    emit requestTaskList();
}

// 当后台工作线程把数据查完并通过信号发过来时，此槽函数在 GUI 线程执行
void TestPanel::onTasksReceived(const QList<TestRecord> &tasks)
{
    // 安全清空表格
    m_tblTasks->setRowCount(0);

    // 遍历后台传过来的实体数据，进行界面组件填充
    for(const auto &t : tasks)
    {
        int row = m_tblTasks->rowCount();
        m_tblTasks->insertRow(row);

        // 填充各列数据
        m_tblTasks->setItem(row, 0, new QTableWidgetItem(QString::number(t.id)));
        m_tblTasks->setItem(row, 1, new QTableWidgetItem(t.name));
        m_tblTasks->setItem(row, 2, new QTableWidgetItem(TestRecord::statusToString(t.status)));
        m_tblTasks->setItem(row, 3, new QTableWidgetItem(t.startTime.toString("yyyy-MM-dd hh:mm:ss")));
        m_tblTasks->setItem(row, 4, new QTableWidgetItem(t.endTime.isValid() ? t.endTime.toString("yyyy-MM-dd hh:mm:ss") : "-"));
        m_tblTasks->setItem(row, 5, new QTableWidgetItem(QString::number(t.sampleCount)));
    }
}