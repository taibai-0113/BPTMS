#include "AlarmPanel.h"
#include "database/DatabaseManager.h"
#include "database/AlarmLogDao.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QBrush>
#include <QColor>
#include <memory>
#include <algorithm>

#include <QDebug>
#include <QMessageBox>

AlarmPanel::AlarmPanel(QWidget *parent) : QWidget(parent) { setupUI(); }

void AlarmPanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8,8,8,8);
    root->setSpacing(6);

    auto *hBar = new QHBoxLayout;
    m_lblCount = new QLabel(tr("未确认告警：0"), this);
    m_lblCount->setStyleSheet("color:#e74c3c; font-weight:bold;");
    m_btnAck = new QPushButton(tr("确认所选警告"), this);
    m_btnAckAll = new QPushButton(tr("确认所有警告"), this);
    m_btnClear = new QPushButton(tr("清空显示"), this);
    hBar->addWidget(m_lblCount);
    hBar->addStretch();
    hBar->addWidget(m_btnAck);
    hBar->addWidget(m_btnAckAll);
    hBar->addWidget(m_btnClear);
    root->addLayout(hBar);

    m_table = new QTableWidget(0, 6, this);
    QStringList headers;
    headers << "告警时间" << "告警内容" << "当前值" << "阈值" << "Id"<< "状态";
    m_table->verticalHeader()->setVisible(false); // 隐藏垂直表头（左上角的全选按钮）
    m_table->setHorizontalHeaderLabels(headers);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    root->addWidget(m_table, 1);

    connect(m_btnAck, &QPushButton::clicked, this, &AlarmPanel::onAckSelected);
    connect(m_btnAckAll, &QPushButton::clicked, this, &AlarmPanel::onAckAll);
    connect(m_btnClear, &QPushButton::clicked, this, &AlarmPanel::clearAlarms);
}

// 专门用于运行时收到单条新告警
void AlarmPanel::addAlarm(const AlarmInfo &info)
{
    if (!m_alarmAll) { // clearAlarms 后 m_alarmAll 会变成空指针，直接 prepend 会崩溃
        m_alarmAll = std::make_unique<QList<AlarmInfo>>();
    }

    // 同步底层数据：新告警加到列表末尾（或开头，取决于你的排序设计）
    m_alarmAll->prepend(info); // 如果界面第0行是最新的，数据也插到最前面

    // 界面插入新行（虽然 insertRow(0) 有开销，但单条插入时人眼感觉不到）
    m_table->insertRow(0);

    auto createItem = [&](const QString &s, bool isAlarm = false) {
        auto *it = new QTableWidgetItem(s);
        it->setTextAlignment(Qt::AlignCenter);
        if (isAlarm) {
            it->setForeground(QBrush(QColor(0xe7, 0x4c, 0x3c)));
            it->setBackground(QBrush(QColor(0x2d, 0x1a, 0x1a)));
        }
        return it;
    };

    m_table->setItem(0, 0, createItem(info.timestamp.toString("MM-dd hh:mm:ss")));
    m_table->setItem(0, 1, createItem(info.typeStr, true));
    m_table->setItem(0, 2, createItem(QString::number(info.actualValue, 'f', 3), true));
    m_table->setItem(0, 3, createItem(QString::number(info.threshold, 'f', 3)));
    m_table->setItem(0, 4, createItem(info.id >= 0
                                          ? QString::number(info.id) : tr("-")));
    m_table->setItem(0, 5, createItem(tr("未确认"), true));

    // 3. 更新未确认计数
    ++m_unacked;
    m_lblCount->setText(QStringLiteral("未确认告警：%1").arg(m_unacked));

    // 持 500 行上限，UI和数据一起删！
    if (m_table->rowCount() > 500) {
        m_table->removeRow(500);     // 删掉界面最底下（最老）的那一行
        m_alarmAll->removeLast();    // 同步删掉底层数据列表里的最老记录
    }
}

// 选中单元格如果该单元格内容是“未确认”，则改为“已确认”，文字颜色改为绿色（#27ae60），清除背景色
// 并将未确认计数 m_unacked 减1。
void AlarmPanel::onAckSelected()
{
    // ranges 返回的是 QList<QTableWidgetSelectionRange> 类型
    // 获取表格中当前选中的单元格区域列表
    const auto ranges = m_table->selectedRanges();
    if (ranges.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请先从列表中选择需要确认的警告！"));
        return;
    }

    // 获取数据库实例并开启事务
    QSqlDatabase db = DatabaseManager::instance()->database();
    AlarmLogDao dao(db);

    // 开启事务：将接下来的所有数据库操作合并为一次磁盘提交，极大地减少I/O开销
    if (!db.transaction()) { // 事务开启失败时不要继续更新数据库
        return;
    }

    bool dbOk = true; // 记录数据库更新是否成功

    for (const auto &range : ranges) {
        for (int row = range.topRow(); row <= range.bottomRow(); ++row) {
            // 获取第5列
            QTableWidgetItem *statusItem = m_table->item(row, 5);

            // 检查盖单元格是否存在且内容未"未确认"
            if (statusItem && statusItem->text() == tr("未确认")) {
                // 获取 ID
                QTableWidgetItem *idItem = m_table->item(row, 4);
                if (!idItem || idItem->text() == tr("-")) continue; // 防止 "-" 被 toInt 后变成 0，误更新 task_id=0

                int alarmId = idItem->text().toInt();

                // 精准更新数据库（因为开启了事务，这里只是写到内存缓存，不会产生多次磁盘开销）
                if (!dao.updateAckStatus(alarmId, 1)) { // 判断数据库是否更新成功
                    dbOk = false;
                    continue;
                }

                // 更新 UI 状态
                statusItem->setText(tr("已确认"));
                statusItem->setForeground(QBrush(QColor(0x27, 0xae, 0x60)));
                statusItem->setBackground(QBrush());

                if (m_unacked > 0) {
                    --m_unacked;
                }

                // 更新内存中的 QList 数据
                if (m_alarmAll) {
                    auto alarmIt = std::find_if(m_alarmAll->begin(), m_alarmAll->end(),
                                                [alarmId](const AlarmInfo& info) {
                                                    return info.id == alarmId;
                                                });
                    if (alarmIt != m_alarmAll->end()) {
                        alarmIt->acknowledged = 1;
                    }
                }
            }
        }
    }

    // 提交事务：一次性写入磁盘
    if (dbOk) { // 数据库操作全部成功才提交
        db.commit();
    } else {
        db.rollback(); // 数据库更新失败时回滚
    }

    m_lblCount->setText(QStringLiteral("未确认告警：%1").arg(m_unacked));
}

// 所有单元格如果该单元格内容是“未确认”，则改为“已确认”，文字颜色改为绿色（#27ae60），清除背景色
// 并将未确认计数 m_unacked 直接变为0。
void AlarmPanel::onAckAll()
{

    // 1. 二次确认弹窗
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  tr("二次确认"),
                                  tr("确定要将所有未确认的警告全部转为已确认吗？"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No); // 默认焦点在“否”上，防止狂点回车误操作

    // 2. 如果用户取消了，直接拦截
    if (reply != QMessageBox::Yes) {
        return;
    }

    // 同步更新数据库
    QSqlDatabase db = DatabaseManager::instance()->database();
    AlarmLogDao dao(db);
    // 数据库更新失败时不继续改 UI 和内存，避免显示和数据库不一致
    if (!dao.ackAllAlarms(m_currentTaskId)) {
        return;
    }

    // 遍历表格更新 UI 状态
    for (int row = 0; row < m_table->rowCount(); ++row) {
        QTableWidgetItem *statusItem = m_table->item(row, 5);
        // 只有是"未确认"的单元格，才进行修改
        if (statusItem && statusItem->text() == tr("未确认")) {
            statusItem->setText(tr("已确认"));
            statusItem->setForeground(QBrush(QColor(0x27, 0xae, 0x60))); // 绿色
            statusItem->setBackground(QBrush()); // 消除背景色
        }
    }

    // 同步更新底层内存数据列表
    if (m_alarmAll) {
        // 直接把所有底层数据的状态改为true
        for (auto &info : *m_alarmAll) {
            info.acknowledged = 1;
        }
    }

    // 更新内存计数器
    m_unacked = 0;
    if (m_lblCount) {
        m_lblCount->setText(tr("未确认告警：0"));
    }
}

void AlarmPanel::loadFromDb(int taskId)
{
    if(!DatabaseManager::instance()->isOpen()) return;
    m_currentTaskId = taskId; // 缓存当前的过滤条件
    AlarmLogDao dao(DatabaseManager::instance()->database());
    m_alarmAll = std::make_unique<QList<AlarmInfo>>(
        (taskId >= 0) ? dao.queryByTask(taskId) : dao.queryAll()
        );

    m_table->setRowCount(0);
    loadAllAlarms();
}

// 专门用于程序启动，或者切换页面时的数据加载
void AlarmPanel::loadAllAlarms()
{
    m_unacked = 0; // 即使没有数据，也要把未确认计数清零
    if (!m_alarmAll || m_alarmAll->isEmpty()) {
        if (m_lblCount) {
            m_lblCount->setText(QStringLiteral("未确认告警：0")); // 空数据时同步刷新显示
        }
        return;
    }

    m_table->setUpdatesEnabled(false);
    m_table->blockSignals(true);

    int totalCount = qMin(m_alarmAll->size(), 500); // 最多只加载500条
    m_table->setRowCount(totalCount);

    // 每次重新加载所有数据时，必须把未确认计数清零
    m_unacked = 0;

    auto createItem = [&](const QString &s, bool isAlarm = false, bool isAcked = false) {
        auto *it = new QTableWidgetItem(s);
        it->setTextAlignment(Qt::AlignCenter);

        if (isAlarm) {
            if (!isAcked) {
                // 未确认状态：红字暗红底
                it->setForeground(QBrush(QColor(0xe7, 0x4c, 0x3c)));
                it->setBackground(QBrush(QColor(0x2d, 0x1a, 0x1a)));
            } else {
                // 已确认状态：绿字透明底
                it->setForeground(QBrush(QColor(0x27, 0xae, 0x60)));
                it->setBackground(QBrush());
            }
        }
        return it;
    };

    for (int row = 0; row < totalCount; ++row) {
        const auto &info = m_alarmAll->at(row);

        // 读取数据库中真实的确认状态
        bool acked = info.acknowledged;
        if (!acked) {
            m_unacked++; // 统计未确认的数量
        }

        m_table->setItem(row, 0, createItem(info.timestamp.toString("MM-dd hh:mm:ss")));
        m_table->setItem(row, 1, createItem(info.typeStr, true, acked)); // 如果你想让其他列在确认后也变绿，可以传入 acked
        m_table->setItem(row, 2, createItem(QString::number(info.actualValue, 'f', 3), true, acked));
        m_table->setItem(row, 3, createItem(QString::number(info.threshold, 'f', 3)));
        m_table->setItem(row, 4, createItem(info.id >= 0 ? QString::number(info.id) : tr("-")));

        // 第5列根据真实状态显示文本和颜色
        QString statusText = acked ? tr("已确认") : tr("未确认");
        m_table->setItem(row, 5, createItem(statusText, true, acked));
    }

    // 重新加载完数据后，更新底部的 UI 计数器
    if (m_lblCount) {
        m_lblCount->setText(QStringLiteral("未确认告警：%1").arg(m_unacked));
    }

    m_table->blockSignals(false);
    m_table->setUpdatesEnabled(true);
}

void AlarmPanel::clearAlarms()
{
    m_alarmAll = nullptr;
    m_table->setRowCount(0);
    m_unacked = 0;
    m_lblCount->setText(tr("未确认告警：0"));
}