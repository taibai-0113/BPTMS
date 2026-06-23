#include "DeviceConfigDialog.h"
#include "common/config/AppConfig.h"
#include "communication/SerialPortManager.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>

// 在对话框中保存数据
DeviceConfigDialog::DeviceConfigDialog(QWidget *parent) : QDialog(parent)
{
    // 设置窗口图标（左上角 + 任务栏）
    QIcon icon(":/icon/serialPortSettings_black.svg");   // 从资源文件加载
    setWindowIcon(icon);
    setWindowTitle(tr("设备配置"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 删除对话框的？
    setMinimumWidth(300);
    setupUI();
}

void DeviceConfigDialog::setupUI()
{
    auto *root = new QVBoxLayout(this);
    AppConfig *cfg = AppConfig::instance();

    auto *grp  = new QGroupBox(tr("串口参数"), this);
    auto *form = new QFormLayout(grp);

    auto *m_lblCbPort = new QLabel(tr("串    口："), this);
    m_cbPort = new QComboBox(this);
    m_cbPort->addItems(SerialPortManager::availablePorts());
    m_cbPort->setCurrentText(cfg->serialPort());

    auto *m_lblCbBaud = new QLabel(tr("波 特 率："), this);
    m_cbBaud = new QComboBox(this);
    m_cbBaud->addItems({"9600","19200","38400","57600","115200"});
    m_cbBaud->setCurrentText(QString::number(cfg->baudRate()));

    auto *m_lblSpnSlave = new QLabel(tr("从 站 ID："), this);
    m_spnSlave = new QSpinBox(this);
    m_spnSlave->setRange(1, 247);
    m_spnSlave->setValue(cfg->slaveId());

    m_chkSim = new QCheckBox(tr("启用模拟器"), this);
    m_chkSim->setChecked(cfg->simulatorMode());

    auto *m_lblSpnInterval = new QLabel(tr("采样间隔："), this);
    m_spnInterval = new QSpinBox(this);
    m_spnInterval->setRange(200, 10000);
    m_spnInterval->setSingleStep(100);
    m_spnInterval->setSuffix(" ms");
    m_spnInterval->setValue(cfg->sampleInterval());

    form->addRow(m_lblCbPort,    m_cbPort);
    form->addRow(m_lblCbBaud,  m_cbBaud);
    form->addRow(m_lblSpnSlave,  m_spnSlave);
    form->addRow(m_lblSpnInterval,m_spnInterval);
    form->addRow(m_chkSim);

    m_lblCbPort->setProperty("stat", true);
    m_lblCbBaud->setProperty("stat", true);
    m_lblSpnSlave->setProperty("stat", true);
    m_lblSpnInterval->setProperty("stat", true);

    root->addWidget(grp);

    auto *btns = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(btns->button(QDialogButtonBox::Save), &QPushButton::clicked,
            this, &DeviceConfigDialog::onAccept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(btns);
}

// 保存数据
void DeviceConfigDialog::onAccept()
{
    AppConfig *cfg = AppConfig::instance();
    cfg->setSerialPort   (m_cbPort->currentText());
    cfg->setBaudRate     (m_cbBaud->currentText().toInt());
    cfg->setSlaveId      (m_spnSlave->value());
    cfg->setSimulatorMode(m_chkSim->isChecked());
    cfg->setSampleInterval(m_spnInterval->value());
    cfg->saveIni();
    accept();
}