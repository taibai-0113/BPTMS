#include "MonitorPanel.h"
#include "ui/widgets/DataCard.h"
#include "ui/widgets/CellVoltageBar.h"
#include "common/config/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QGroupBox>

MonitorPanel::MonitorPanel(QWidget *parent) : QWidget(parent)
{
    m_cfg = AppConfig::instance()->alarmConfig(); // 先获取好告警
    setupUI();
}

void MonitorPanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    // ── 顶部：时间戳 + 状态标语 ──────────────────────
    auto *hTop = new QHBoxLayout;
    m_lblTimestamp = new QLabel(tr("等待数据..."), this);
    m_lblTimestamp->setStyleSheet("color:#666; font-size:11px;");
    m_lblChargeStatus = new QLabel(tr("状态: —"), this);
    m_lblChargeStatus->setStyleSheet("color:#27ae60; font-weight:bold;");
    m_lblAlarmBanner  = new QLabel(tr(""), this);
    m_lblAlarmBanner->setStyleSheet(
        "color:#e74c3c; font-weight:bold; font-size:13px; "
        "background-color:#2d1a1a; padding:2px 8px; border-radius:3px;");
    m_lblAlarmBanner->hide();
    hTop->addWidget(m_lblTimestamp);
    hTop->addStretch();
    hTop->addWidget(m_lblAlarmBanner);
    hTop->addWidget(m_lblChargeStatus);
    root->addLayout(hTop);

    // ── 中部：数值卡片区域 ────────────────────────────
    auto *grpCards = new QGroupBox(tr("实时数据"), this);
    auto *hCards = new QHBoxLayout(grpCards);
    hCards->setSpacing(8);
    // auto *gridCards = new QGridLayout(grpCards);
    // gridCards->setSpacing(8);

    m_cardVoltage = new DataCard(tr("总电压"),  "V",   this);
    m_cardCurrent = new DataCard(tr("总电流"),  "A",   this);
    m_cardSOC     = new DataCard(tr("SOC"),    "%",   this);
    m_cardSOH     = new DataCard(tr("SOH"),    "%",   this);
    m_cardTemp1   = new DataCard(tr("温度 1"), "°C",  this);
    m_cardTemp2   = new DataCard(tr("温度 2"), "°C",  this);
    m_cardTemp3   = new DataCard(tr("温度 3"), "°C",  this);
    m_cardPower   = new DataCard(tr("功率"),   "W",   this);
    m_cardCycles  = new DataCard(tr("循环次数"),"次", this);

    // 3×3 布局
    // gridCards->addWidget(m_cardVoltage, 0, 0);
    // gridCards->addWidget(m_cardCurrent, 0, 1);
    // gridCards->addWidget(m_cardPower,   0, 2);
    // gridCards->addWidget(m_cardSOC,     1, 0);
    // gridCards->addWidget(m_cardSOH,     1, 1);
    // gridCards->addWidget(m_cardCycles,  1, 2);
    // gridCards->addWidget(m_cardTemp1,   2, 0);
    // gridCards->addWidget(m_cardTemp2,   2, 1);
    // gridCards->addWidget(m_cardTemp3,   2, 2);

    // 水平布局
    hCards->addWidget(m_cardVoltage);
    hCards->addWidget(m_cardCurrent);
    hCards->addWidget(m_cardPower);
    hCards->addWidget(m_cardSOC);
    hCards->addWidget(m_cardSOH);
    hCards->addWidget(m_cardCycles);
    hCards->addWidget(m_cardTemp1);
    hCards->addWidget(m_cardTemp2);
    hCards->addWidget(m_cardTemp3);

    root->addWidget(grpCards, 2);

    // ── 图表区域（Tab 切换）──────────────────────────────
    m_chartTab = new QTabWidget(this);

    // 电压曲线
    m_chartVoltage = new ScrollingChart(this);
    m_chartVoltage->setTitle("总电压 (V)");
    m_chartVoltage->setYRange(20, 30);
    m_chartVoltage->addSeries("电压", QColor(0x3a, 0x9b, 0xdc));
    m_chartVoltage->addAlarmLine(AppConfig::instance()->alarmConfig().overVoltage,
                                 QColor(0xe7,0x4c,0x3c), "OV");
    m_chartVoltage->addAlarmLine(AppConfig::instance()->alarmConfig().underVoltage,
                                 QColor(0xe6,0x7e,0x22), "UV");

    // 电流曲线
    m_chartCurrent = new ScrollingChart(this);
    m_chartCurrent->setTitle("总电流 (A)");
    m_chartCurrent->setYRange(-40, 40);
    m_chartCurrent->addSeries("电流", QColor(0x27, 0xae, 0x60));
    m_chartCurrent->addAlarmLine(AppConfig::instance()->alarmConfig().overCurrent,
                                 QColor(0xe7,0x4c,0x3c), "OV");
    m_chartCurrent->addAlarmLine(-AppConfig::instance()->alarmConfig().overCurrent,
                                 QColor(0xe7,0x4c,0x3c), "OV");

    // SOC 曲线
    m_chartSOC = new ScrollingChart(this);
    m_chartSOC->setTitle("SOC (%)");
    m_chartSOC->setYRange(0, 100);
    m_chartSOC->setAutoYRange(false);
    m_chartSOC->addSeries("SOC", QColor(0x9b, 0x59, 0xb6));

    // 温度曲线（三路叠加）
    m_chartTemp = new ScrollingChart(this);
    m_chartTemp->setTitle("温度 (°C)");
    m_chartTemp->setYRange(0, 65);
    m_chartTemp->addSeries("T1", QColor(0xe7, 0x4c, 0x3c));
    m_chartTemp->addSeries("T2", QColor(0xe6, 0x7e, 0x22));
    m_chartTemp->addSeries("T3", QColor(0xf3, 0x9c, 0x12));
    m_chartTemp->addAlarmLine(AppConfig::instance()->alarmConfig().overTemp,
                              QColor(0xe7,0x4c,0x3c), "OT");

    m_chartTab->addTab(m_chartVoltage, tr("电压"));
    m_chartTab->addTab(m_chartCurrent, tr("电流"));
    m_chartTab->addTab(m_chartSOC,     tr("SOC"));
    m_chartTab->addTab(m_chartTemp,    tr("温度"));

    root->addWidget(m_chartTab, 5);  // 给图表更多空间

    // ── 单体电压条形图 ────────────────────────────────
    auto *grpCells = new QGroupBox(tr("单体电压（V）"), this);
    auto *layCells = new QVBoxLayout(grpCells);
    m_cellBar = new CellVoltageBar(this);
    m_cellBar->setThresholds(m_cfg.cellOverVolt, m_cfg.cellUnderVolt);
    layCells->addWidget(m_cellBar);
    root->addWidget(grpCells, 3);
}

void MonitorPanel::updateData(const BatteryData &d)
{
    m_lblTimestamp->setText(
        d.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz"));

    // ── 数值卡片更新 ──────────────────────────────────
    m_cardVoltage->setValue(d.totalVoltage, 2);
    m_cardVoltage->setAlarm(d.totalVoltage > m_cfg.overVoltage ||
                            d.totalVoltage < m_cfg.underVoltage);

    m_cardCurrent->setValue(d.totalCurrent, 2);
    m_cardCurrent->setAlarm(std::abs(d.totalCurrent) > m_cfg.overCurrent);

    m_cardSOC->setValue(d.soc, 0);
    m_cardSOH->setValue(d.soh, 0);

    m_cardTemp1->setValue(d.temp1, 1);
    m_cardTemp1->setAlarm(d.temp1 > m_cfg.overTemp);
    m_cardTemp2->setValue(d.temp2, 1);
    m_cardTemp2->setAlarm(d.temp2 > m_cfg.overTemp);
    m_cardTemp3->setValue(d.temp3, 1);
    m_cardTemp3->setAlarm(d.temp3 > m_cfg.overTemp);

    double power = d.totalVoltage * std::abs(d.totalCurrent);
    m_cardPower->setValue(power, 1);
    m_cardCycles->setValue(d.cycleCount, 0);

    // ── 滚轮折线图 ──────────────────────────────────────
    m_chartVoltage->pushValue("电压",  d.totalVoltage);
    m_chartCurrent->pushValue("电流",  std::abs(d.totalCurrent));
    m_chartSOC->pushValue    ("SOC",   static_cast<double>(d.soc));
    m_chartTemp->pushValue   ("T1",    d.temp1);
    m_chartTemp->pushValue   ("T2",    d.temp2);
    m_chartTemp->pushValue   ("T3",    d.temp3);

    // ── 单体电压 ──────────────────────────────────────
    m_cellBar->setVoltages(d.cellVoltages);

    // ── 状态标签 ──────────────────────────────────────
    QString statusStr;
    if (d.isCharging())    statusStr = tr("充电中 ▲");
    else if (d.isDischarging()) statusStr = tr("放电中 ▼");
    else statusStr = tr("待机");

    if (d.isBalancing()) statusStr += tr(" [均衡]");
    m_lblChargeStatus->setText(QStringLiteral("状态: %1").arg(statusStr));

    if (d.hasAlarm()) {
        m_lblAlarmBanner->setText(tr("⚠ 存在活跃告警！"));
        m_lblAlarmBanner->show();
    } else {
        m_lblAlarmBanner->hide();
    }
}

void MonitorPanel::setTimeWindow(int ms)
{
    ms = (ms * 60) / 1000;
    m_chartVoltage->setTimeWindow(ms);
    m_chartCurrent->setTimeWindow(ms);
    m_chartSOC->setTimeWindow(ms);
    m_chartTemp->setTimeWindow(ms);
}

void MonitorPanel::clearDisplay()
{
    // 1. 数值卡片（可选：恢复默认值/空白，避免显示过期数值）
    m_cardVoltage->setValue(0.0, 2);
    m_cardCurrent->setValue(0.0, 2);
    m_cardSOC->setValue(0, 0);
    m_cardSOH->setValue(0, 0);
    m_cardTemp1->setValue(0.0, 1);
    m_cardTemp2->setValue(0.0, 1);
    m_cardTemp3->setValue(0.0, 1);
    m_cardPower->setValue(0.0, 1);
    m_cardCycles->setValue(0, 0);

    // 2. 折线图：清除所有系列的历史点
    m_chartVoltage->clearAll();   // 具体看你的图表类的接口
    m_chartCurrent->clearAll();
    m_chartSOC->clearAll();
    m_chartTemp->clearAll();      // 如果同一个图表内有多个系列，需逐个清除或清空整个图表

    // 3. 单体电压条
    m_cellBar->setVoltages({}); // 或调用 clear() / reset()

    // 4. 状态标签
    m_lblChargeStatus->setText(tr("状态: —"));
    m_lblAlarmBanner->setText(tr(""));
}

void MonitorPanel::setAlarmConfig(const AlarmConfig &cfg)
{
    m_cfg = cfg;
    m_cellBar->setThresholds(cfg.cellOverVolt, cfg.cellUnderVolt);
}