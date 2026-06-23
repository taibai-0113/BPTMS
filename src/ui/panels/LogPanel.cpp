#include "LogPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <QDateTime>
#include <QLabel>

LogPanel::LogPanel(QWidget *parent) : QWidget(parent) {setupUI();}

void LogPanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(4,4,4,4);
    root->setSpacing(4);

    auto *hBar = new QHBoxLayout;
    m_minLevelChk = new QLabel(tr("日志记录阈值："),this);
    m_cbLevel = new QComboBox(this);
    m_cbLevel->addItems({"DEBUG","INFO","WARNING","ERROR","CRITICAL"});
    m_cbLevel->setCurrentIndex(0);
    m_chkAuto = new QCheckBox(tr("自动滚动"), this);
    m_chkAuto->setObjectName("autoScrollCheckBox");
    m_chkAuto->setChecked(true);
    m_btnClear = new QPushButton(tr("清空"), this);
    m_btnSave = new QPushButton(tr("保存"), this);
    hBar->addWidget(m_minLevelChk);
    hBar->addWidget(m_cbLevel);
    hBar->addWidget(m_chkAuto);
    hBar->addStretch();
    hBar->addWidget(m_btnClear);
    hBar->addWidget(m_btnSave);
    root->addLayout(hBar);

    m_editor = new QPlainTextEdit(this);
    m_editor->setReadOnly(true);
    m_editor->setMaximumBlockCount(MAX_LINES);
    m_editor->setFont(QFont("Conslolas", 9));
    m_editor->setStyleSheet(
        "background-color:#0d1117; color:#c9d1d9;"
        "border:1px solid #30363d;");
    root->addWidget(m_editor);

    connect(m_btnClear, &QPushButton::clicked, this, &LogPanel::onClearClicked);
    connect(m_btnSave, &QPushButton::clicked, this, &LogPanel::onSaveClicked);
    connect(m_cbLevel, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogPanel::onLevelChanged);
}

// 往日志面板添加数据，不同日志级别设置不同颜色
void LogPanel::appendLog(const QString &line, LogLevel level)
{
    if(level < m_minLevel) return; // 判断是否小于最低阈值

    QString color;
    switch (level) {
    case LogLevel::DEBUG:    color = "#6a9fb5"; break;
    case LogLevel::INFO:     color = "#c9d1d9"; break;
    case LogLevel::WARNING:  color = "#e6db74"; break;
    case LogLevel::ERR:      color = "#f97583"; break;
    case LogLevel::CRITICAL: color = "#ff0000"; break;
    default:                 color = "#c9d1d9"; break;
    }

    m_editor->appendHtml(
        QStringLiteral("<span style='color:%1;'>%2</span>")
            .arg(color, line.toHtmlEscaped()));

    // 当“自动滚动”复选框被勾选时，将文本编辑器的垂直滚动条自动滚动到底部
    if(m_chkAuto->isChecked())
        m_editor->verticalScrollBar()->setValue(
            m_editor->verticalScrollBar()->maximum());
}

// 清空所有日志数据
void LogPanel::onClearClicked() {m_editor->clear(); m_lineCount = 0;}

// 保存日志
void LogPanel::onSaveClicked()
{
    QString path = QFileDialog::getSaveFileName(
        this, tr("保存日志"),
        QStringLiteral("bptms_log_%1.txt")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        tr("文本文件 (*.txt)"));

    if(path.isEmpty()) return;
    QFile f(path); // 追加数据
    if(f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream ts(&f);
        ts << m_editor->toPlainText();
    }
}

// 最低阈值
void LogPanel::onLevelChanged(int idx)
{
    m_minLevel = static_cast<LogLevel>(idx);
}