#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QSpinBox>
#include <QTextEdit>
#include "core/data/TestRecord.h"

class CreateTaskDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CreateTaskDialog(QWidget *parent = nullptr);
    TestRecord createTask() const;

private:
    void setupUI();

    QTextEdit *m_textTaskName {nullptr};
    QTextEdit *m_textTaskDescription {nullptr};
};