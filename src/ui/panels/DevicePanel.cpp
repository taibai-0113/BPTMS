#include "DevicePanel.h"
#include "common/config/AppConfig.h"
#include "communication/SerialPortManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QTimer>

DevicePanel::DevicePanel(QWidget *parent) : QWidget(parent)
{
    // 初始化UI并刷新串口跟时间
    setupUI();
    refreshPorts();
    m_runTimer = new QTimer(this);
    connect(m_runTimer, &QTimer::timeout, this, [this](){
        ++m_elapsed;
        int h = m_elapsed / 3600, m = (m_elapsed % 3600) / 60, s = m_elapsed % 60;
        m_lblRunTime->setText(QStringLiteral("%1:%2:%3")
                                  .arg(h,2,10,QLatin1Char('0'))
                                  .arg(m,2,10,QLatin1Char('0'))
                                  .arg(s,2,10,QLatin1Char('0')));
    });
}

void DevicePanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(6,6,6,6);
    root->setSpacing(8);

    // ── 连接配置区 ──────────────────────────────────
    auto *grpConn = new QGroupBox(tr("设备连接"), this);
    auto *formConn = new QFormLayout(grpConn);
    formConn->setLabelAlignment(Qt::AlignRight);

    m_chkSimulator = new QCheckBox(tr("使用模拟器"), this);
    m_chkSimulator->setChecked(AppConfig::instance()->simulatorMode());
    formConn->addRow(m_chkSimulator);

    m_cbPort = new QComboBox(this);
    m_cbPort->setEditable(!m_chkSimulator->isChecked()); // 只需要禁用端口号，波特率可以不用禁止
    auto *lblPort = new QLabel(tr("串  口："), this);
    formConn->addRow(lblPort, m_cbPort);
    m_cbPort->setEnabled(false);
    connect(m_chkSimulator, &QCheckBox::toggled,
            m_cbPort, [this](bool sim){m_cbPort->setEnabled(!sim);});

    m_cbBaud = new QComboBox(this);
    m_cbBaud->addItems({"9600", "19200", "388400", "57600", "115200"});
    m_cbBaud->setCurrentText(QString::number(AppConfig::instance()->baudRate()));
    auto *lblBaud = new QLabel(tr("波特率："), this);
    formConn->addRow(lblBaud, m_cbBaud);

    lblPort->setProperty("stat", true);
    lblBaud->setProperty("stat", true);

    auto *hBtns = new QHBoxLayout;
    m_btnRefresh = new QPushButton(tr("刷新"), this);
    m_btnConnect = new QPushButton(tr("连接"), this);
    m_btnDisconnect = new QPushButton(tr("断开"), this);
    m_btnConnect->setObjectName("btnConnect");
    m_btnDisconnect->setObjectName("btnDisconnect");
    hBtns->addWidget(m_btnRefresh);
    hBtns->addWidget(m_btnConnect);
    hBtns->addWidget(m_btnDisconnect);
    formConn->addRow(hBtns);

    m_lblStatus = new QLabel(tr("● 未连接"), this);
    m_lblStatus->setObjectName("labelDisconnected");
    formConn->addRow(m_lblStatus);

    root->addWidget(grpConn);

    // ── 采集设置区 ──────────────────────────────────
    auto *grpPoll = new QGroupBox(tr("采集设置"), this);
    auto *formPoll = new QFormLayout(grpPoll);
    m_spnInterval = new QSpinBox(this);
    m_spnInterval->setRange(200, 10000);
    m_spnInterval->setSingleStep(100);
    m_spnInterval->setSuffix(" ms");
    m_spnInterval->setValue(AppConfig::instance()->sampleInterval());
    auto *lblInterval = new QLabel(tr("采样间隔："), this);

    formPoll->addRow(lblInterval, m_spnInterval);
    lblInterval->setProperty("stat", true);
    m_spnInterval->setProperty("stat", true);

    root->addWidget(grpPoll);

    // ── 运行状态区 ──────────────────────────────────
    auto *grpStat = new QGroupBox(tr("运行状态"), this);
    auto *formStat = new QFormLayout(grpStat);
    m_lblFrameCount = new QLabel("0", this);
    m_lblQuality = new QLabel("-", this);
    m_lblRunTime = new QLabel("00:00:00", this);
    auto *lblFrame = new QLabel(tr("帧 计 数："), this);
    auto *lblQuality = new QLabel(tr("通信质量："), this);
    auto *lblRunTime = new QLabel(tr("运行时长："), this);

    formStat->addRow(lblFrame, m_lblFrameCount);
    formStat->addRow(lblQuality, m_lblQuality);
    formStat->addRow(lblRunTime, m_lblRunTime);
    lblFrame->setProperty("stat", true);
    lblQuality->setProperty("stat", true);
    lblRunTime->setProperty("stat", true);
    m_lblFrameCount->setProperty("stat", true);
    m_lblQuality->setProperty("stat", true);
    m_lblRunTime->setProperty("stat", true);

    root->addWidget(grpStat);

    root->addStretch();

    // ── 信号连接 ────────────────────────────────────
    connect(m_btnRefresh, &QPushButton::clicked, this, &DevicePanel::refreshPorts);
    connect(m_btnConnect, &QPushButton::clicked, this, &DevicePanel::onConnectClicked);
    connect(m_btnDisconnect, &QPushButton::clicked,
            this, &DevicePanel::disconnectRequested);
    connect(m_spnInterval, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DevicePanel::intervalChanged);
}

void DevicePanel::onConnectClicked()
{
    emit connectRequested(
        m_chkSimulator->isChecked(),
        m_cbPort->currentText(),
        m_cbBaud->currentText().toInt());
}

void DevicePanel::refreshPorts()
{
    m_cbPort->clear();
    m_cbPort->addItems(SerialPortManager::availablePorts());
    if(m_cbPort->count() == 0)m_cbPort->addItem(tr("（无可用串口）"));
}

// 连接设备后更新状态，并禁用连接按钮跟使用模拟器按钮，放置发送歧义
void DevicePanel::setConnected(bool connected, const QString &deviceName)
{
    m_connected = connected;
    m_btnConnect->setEnabled(!connected);
    m_btnDisconnect->setEnabled(connected);
    m_chkSimulator->setEnabled(!connected);
    m_cbPort->setEnabled(!connected && !m_chkSimulator->isChecked());

    if(connected)
    {
        m_lblStatus->setText(QStringLiteral("● 已连接: %1").arg(deviceName));
        m_lblStatus->setObjectName("labelConnected");
        m_elapsed = 0;
        m_runTimer->start(1000);
    }else{
        m_lblStatus->setText(tr("● 未连接"));
        m_lblStatus->setObjectName("labelDisconnected");
        m_runTimer->stop();
    }

    // 强制刷新控件的样式，确保样式表或动态属性更改后立即生效
    m_lblStatus->style()->unpolish(m_lblStatus); // 清除样式
    m_lblStatus->style()->polish(m_lblStatus); // 使用样式
}

// 更新设备面板的运行状态区
void DevicePanel::updateFrameStats(int total, int failed)
{
    m_lblFrameCount->setText(QString::number(total));
    if (total > 0) {
        double quality = (1.0 - (double)failed / total) * 100.0;
        m_lblQuality->setText(QStringLiteral("%1%").arg(quality, 0, 'f', 1));
    }
}
