#pragma once

#include <QFrame>
#include <QLabel>

// 单个数值显示卡片：标题 + 大数字 + 单位
class DataCard : public QFrame{
    Q_OBJECT
public:
    explicit DataCard(const QString &title,
                      const QString &unit,
                      QWidget *parent = nullptr);

    void setValue(double val, int decimals = 2);
    void setValue(const QString &text);
    void setAlarm(bool alarm);

private:
    QLabel *m_lblTitle;
    QLabel *m_lblValue;
    QLabel *m_lblUnit;
    bool m_alarm { false };
};