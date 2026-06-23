#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include "common/logger/Logger.h"

class LogPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LogPanel(QWidget *parent = nullptr);

public slots:
    void appendLog(const QString &line, LogLevel level);

private slots:
    void onClearClicked(); // 清空
    void onSaveClicked(); // 保存
    void onLevelChanged(int idx);

private:
    void setupUI();

    QLabel *m_minLevelChk {nullptr};
    QPlainTextEdit *m_editor {nullptr};
    QComboBox *m_cbLevel {nullptr};
    QPushButton *m_btnClear {nullptr};
    QPushButton *m_btnSave {nullptr};
    QCheckBox *m_chkAuto {nullptr};
    LogLevel m_minLevel {LogLevel::DEBUG};
    int m_lineCount {0};
    static const int MAX_LINES = 2000;
};