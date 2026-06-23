#include "HistoryPanel.h"
#include "database/DatabaseManager.h"
#include "database/TaskDao.h"
#include "database/SampleDataDao.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

HistoryPanel::HistoryPanel(QWidget *parent) : QWidget(parent)
{
    setupUI();
    refreshTaskList();
}

void HistoryPanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8,8,8,8);
    root->setSpacing(8);

    // ── 查询工具栏 ───────────────────────────────────
    auto *hBar =new QHBoxLayout;
    m_cbTask = new QComboBox(this);
    m_cbTask->setMinimumWidth(200);
    m_btnRefresh = new QPushButton(tr("刷新"), this);
    m_btnQuery = new QPushButton(tr("查询"), this);
    m_btnExport = new QPushButton(tr("导出CSV"), this);
    m_lblCount = new QLabel(tr("共 0 条"), this);
    m_lblCount->setStyleSheet("color:#888;");
    hBar->addWidget(new QLabel(tr("任务："), this));
    hBar->addWidget(m_cbTask, 2);
    hBar->addWidget(m_btnRefresh);
    hBar->addWidget(m_btnQuery);
    hBar->addStretch();
    hBar->addWidget(m_lblCount);
    hBar->addWidget(m_btnExport);
    root->addLayout(hBar);

    // ── 数据表格 ─────────────────────────────────────
    m_table = new QTableWidget(0, 9, this);
    m_table->setHorizontalHeaderLabels({
        tr("时间"), tr("电压(V)"), tr("电流(A)"),
        tr("SOC(%)"), tr("SOH(%)"),
        tr("T1(°C)"), tr("T2(°C)"), tr("T3(°C)"),
        tr("循环")
    });
    m_table->verticalHeader()->setVisible(false); // 隐藏垂直表头（左上角的全选按钮）
    // 设置所有列的宽度模式为拉伸，当表格总宽度变化时，各列按比例填满可用空间
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // 第0列的宽度根据内容自动调整
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // 第 1 列负责填满剩余空间
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows); // 点击任意单元格即选中整行
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);// 表格内容不可编辑
    m_table->setAlternatingRowColors(true); // 交替行背景色（奇数偶数行颜色不同）
    root->addWidget(m_table, 1);

    connect(m_btnRefresh, &QPushButton::clicked, this, &HistoryPanel::refreshTaskList);
    connect(m_btnQuery,   &QPushButton::clicked, this, &HistoryPanel::onQueryClicked);
    connect(m_btnExport,  &QPushButton::clicked, this, &HistoryPanel::onExportClicked);
}

// 刷新列表，从数据库中更新数据
void HistoryPanel::refreshTaskList()
{
    m_cbTask->clear();
    if(!DatabaseManager::instance()->isOpen()) return;

    TaskDao dao(DatabaseManager::instance()->database());
    for(const auto &t : dao.queryAll())
    {
        m_cbTask->addItem(
            QStringLiteral("#%1 %2 [%3]")
                .arg(t.id).arg(t.name)
                .arg(TestRecord::statusToString(t.status)));
    }
}

// 发送Id
void HistoryPanel::onQueryClicked()
{
    QString text = m_cbTask->currentText();
    QRegularExpression re("#(\\d+)");
    QRegularExpressionMatch match = re.match(text);
    currentId = -1;

    if (match.hasMatch()) {
        currentId = match.captured(1).toInt();
    }

    TaskDao dao(DatabaseManager::instance()->database(), this);
    int maxId = dao.getMaxId();

    if(currentId < 0 || currentId > maxId)return;
    emit queryRequested(currentId);
}

// 将数据放到历史表格中
void HistoryPanel::displayData(const QList<BatteryData> &list)
{
    m_table->setRowCount(0);
    for(const auto &d : list)
    {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        auto item = [&](const QString &s){
            auto *it = new QTableWidgetItem(s);
            it->setTextAlignment(Qt::AlignCenter); // 将表格设置为居中对齐
            return it;
        };
        // 给每一行插入数据
        m_table->setItem(row, 0, item(d.timestamp.toString("hh:mm:ss.zzz")));
        m_table->setItem(row, 1, item(QString::number(d.totalVoltage, 'f', 3)));
        m_table->setItem(row, 2, item(QString::number(d.totalCurrent, 'f', 3)));
        m_table->setItem(row, 3, item(QString::number(d.soc)));
        m_table->setItem(row, 4, item(QString::number(d.soh)));
        m_table->setItem(row, 5, item(QString::number(d.temp1, 'f', 1)));
        m_table->setItem(row, 6, item(QString::number(d.temp2, 'f', 1)));
        m_table->setItem(row, 7, item(QString::number(d.temp1, 'f', 1)));
        m_table->setItem(row, 8, item(QString::number(d.cycleCount)));
    }
    m_lblCount->setText(QStringLiteral("共 %1 条").arg(list.size()));
    // m_table->resizeColumnsToContents();
}

// 导出为CSV
void HistoryPanel::onExportClicked()
{
    TaskDao TaskDao(DatabaseManager::instance()->database(), this);
    int maxId = TaskDao.getMaxId();

    if(currentId < 0 || currentId > maxId)return;

    QString path = QFileDialog::getSaveFileName(
        this, tr("导出 CSV"),
        QStringLiteral("task_%1.csv").arg(currentId),
        tr("CSV 文件 (*.csv)"));
    if(path.isEmpty())return;

    SampleDataDao dao(DatabaseManager::instance()->database());
    QString csv = dao.exportCsv(currentId); // 把数据导出为CSV

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("导出失败"), f.errorString());
        return;
    }
    QTextStream ts(&f); // 数据中转流
    ts << csv;
    QMessageBox::information(this, tr("导出成功"),
                             QStringLiteral("CSV 已保存至:\n%1").arg(path));
}