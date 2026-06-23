#include "CreateTaskDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

CreateTaskDialog::CreateTaskDialog(QWidget *parent)
    : QDialog(parent)
{
    // 设置窗口图标（左上角 + 任务栏）
    QIcon icon(":/icon/createTaskDialog.svg");   // 从资源文件加载
    setWindowIcon(icon);
    setWindowTitle(tr("任务创建"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 删除对话框的？
    setMinimumWidth(240);
    setupUI();
}

void CreateTaskDialog::setupUI()
{
    auto *root = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    m_textTaskName = new QTextEdit(this);
    m_textTaskDescription = new QTextEdit(this);

    form->addRow(tr("任务名称："), m_textTaskName);
    form->addRow(tr("任务描述："), m_textTaskDescription);

    root->addLayout(form);
    setLayout(root);

    auto *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(btns);
}
TestRecord CreateTaskDialog::createTask()const
{
    TestRecord t;
    t.name   = m_textTaskName->toPlainText();
    t.description  = m_textTaskDescription->toPlainText();
    return t;
}