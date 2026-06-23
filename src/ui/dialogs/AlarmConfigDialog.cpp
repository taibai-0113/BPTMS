#include "AlarmConfigDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QIcon>

// 在mainwindow中保存数据（因为还需要将数据穿个alarmChecker）
AlarmConfigDialog::AlarmConfigDialog(const AlarmConfig &current, QWidget *parent)
    : QDialog(parent)
{
    // 设置窗口图标（左上角 + 任务栏）
    QIcon icon(":/icon/alarmConfig_black.svg");   // 从资源文件加载
    setWindowIcon(icon);
    setWindowTitle(tr("告警阈值配置"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 删除对话框的？
    setMinimumWidth(320);
    setupUI();
    loadConfig(current);
}

void AlarmConfigDialog::setupUI()
{
    auto *root = new QVBoxLayout(this);

    auto makeSpn = [](double min, double max, double step, int dec){
        auto *spn = new QDoubleSpinBox;
        spn->setRange(min, max);
        spn->setSingleStep(step);
        spn->setDecimals(dec);
        return spn;
    };

    auto *grpPack = new QGroupBox(tr("电池包"), this);
    auto *formPack = new QFormLayout(grpPack);
    m_spnOverVoltage  = makeSpn(20, 30, 0.1, 2); // 最大值、最小值、步长、小数位
    m_spnUnderVoltage = makeSpn(20, 30, 0.1, 2);
    m_spnOverCurrent  = makeSpn(0,  40, 0.5, 1);
    m_spnOverTemp     = makeSpn(0, 65,  0.5, 1);
    m_spnOverVoltage->setSuffix("  V");
    m_spnUnderVoltage->setSuffix(" V");
    m_spnOverCurrent->setSuffix("  A");
    m_spnOverTemp->setSuffix("    °C");

    auto *m_lblSpnOverVoltage = new QLabel(tr("过压阈值："), this);
    auto *m_lblSpnUnderVoltage = new QLabel(tr("欠压阈值："), this);
    auto *m_lblSpnOverCurren = new QLabel(tr("过流阈值："), this);
    auto *m_lblSpnOverTemp = new QLabel(tr("过温阈值："), this);

    formPack->addRow(m_lblSpnOverVoltage, m_spnOverVoltage);
    formPack->addRow(m_lblSpnUnderVoltage, m_spnUnderVoltage);
    formPack->addRow(m_lblSpnOverCurren, m_spnOverCurrent);
    formPack->addRow(m_lblSpnOverTemp, m_spnOverTemp);

    m_lblSpnOverVoltage->setProperty("stat", true);
    m_lblSpnUnderVoltage->setProperty("stat", true);
    m_lblSpnOverCurren->setProperty("stat", true);
    m_lblSpnOverTemp->setProperty("stat", true);

    root->addWidget(grpPack);

    auto *grpCell = new QGroupBox(tr("单体电池"), this);
    auto *formCell = new QFormLayout(grpCell);
    m_spnCellOverVolt = makeSpn(2.8, 3.6, 0.01, 3);
    m_spnCellUnderVolt = makeSpn(2.8, 3.6, 0.01, 3);
    m_spnCellOverVolt->setSuffix("  V");
    m_spnCellUnderVolt->setSuffix(" V");

    auto *m_lblSpnCellOverVolt = new QLabel(tr("单体过压："), this);
    auto *m_lblSpnCellUnderVolt = new QLabel(tr("单体欠压："), this);

    formCell->addRow(m_lblSpnCellOverVolt, m_spnCellOverVolt);
    formCell->addRow(m_lblSpnCellUnderVolt, m_spnCellUnderVolt);

    m_lblSpnCellOverVolt->setProperty("stat", true);
    m_lblSpnCellUnderVolt->setProperty("stat", true);

    root->addWidget(grpCell);

    auto *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(btns);
}

// 加载数据
void AlarmConfigDialog::loadConfig(const AlarmConfig &c)
{
    m_spnOverVoltage->setValue(c.overVoltage);
    m_spnUnderVoltage->setValue(c.underVoltage);
    m_spnOverCurrent->setValue(c.overCurrent);
    m_spnOverTemp->setValue(c.overTemp);
    m_spnCellOverVolt->setValue(c.cellOverVolt);
    m_spnCellUnderVolt->setValue(c.cellUnderVolt);
}

// 返回保存数据
AlarmConfig AlarmConfigDialog::alarmConfig()const
{
    AlarmConfig c;
    c.overVoltage   = m_spnOverVoltage->value();
    c.underVoltage  = m_spnUnderVoltage->value();
    c.overCurrent   = m_spnOverCurrent->value();
    c.overTemp      = m_spnOverTemp->value();
    c.cellOverVolt  = m_spnCellOverVolt->value();
    c.cellUnderVolt = m_spnCellUnderVolt->value();
    return c;
}