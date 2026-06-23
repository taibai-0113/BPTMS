#include "DataCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

// 初始化即绘画UI
DataCard::DataCard(const QString &title, const QString &unit, QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setMinimumSize(110, 80);
    setMaximumSize(160, 100);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(8, 6, 8, 6);
    lay->setSpacing(2);

    m_lblTitle = new QLabel(title, this);
    m_lblTitle->setAlignment(Qt::AlignCenter);
    m_lblTitle->setStyleSheet("color:#888; font-size:11px;");

    auto *hLay = new QHBoxLayout;
    m_lblValue = new QLabel("-", this);
    m_lblValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_lblValue->setStyleSheet(
        "color:#3498db; font-size:20px; font-weight:bold;");
    m_lblUnit = new QLabel(unit, this);
    m_lblUnit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_lblUnit->setStyleSheet(
        "color:#aaa; font-size:11px; padding-top:6px;");
    hLay->addStretch(2);
    hLay->addWidget(m_lblValue, 4);
    hLay->addWidget(m_lblUnit, 2);
    hLay->addStretch(1);

    lay->addWidget(m_lblTitle);
    lay->addLayout(hLay);
}

// 放置数据
void DataCard::setValue(double val, int decimals)
{
    m_lblValue->setText(QString::number(val, 'f', decimals));
}

// 放置数据
void DataCard::setValue(const QString &text)
{
    m_lblValue->setText(text);
}

// 出现告警需要变色
void DataCard::setAlarm(bool alarm)
{
    if (m_alarm == alarm) return;
    m_alarm = alarm;

    if (alarm) {
        // 告警状态
        m_lblTitle->setStyleSheet("background-color:#2d1a1a;");
        m_lblValue->setStyleSheet("color:#e74c3c; background-color:#2d1a1a; font-size:20px; font-weight:bold;");
        m_lblUnit->setStyleSheet("background-color:#2d1a1a;");
        setStyleSheet("DataCard { background-color:#2d1a1a; border: 1px solid #e74c3c; }");
    } else {
        // 恢复正常
        m_lblTitle->setStyleSheet("");
        m_lblValue->setStyleSheet("color:#3498db; font-size:20px; font-weight:bold;");
        m_lblUnit->setStyleSheet("");
        setStyleSheet("");
    }
}