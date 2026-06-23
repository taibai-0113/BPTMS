#pragma once

#include <QWidget>
#include <array>

// 8 节单体电压和横条可视化（自定义绘制）
class CellVoltageBar : public QWidget
{
    Q_OBJECT
public:
    explicit CellVoltageBar(QWidget *parent = nullptr);

    // 基本函数
    void setVoltages(const std::array<double, 8> &vols);
    void setThresholds(double over, double under);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    // 绘制电压阈值图
    void paintEvent(QPaintEvent *event) override;

private:
    std::array<double, 8> m_voltages {};
    double m_overThreshold {4.05};
    double m_underThreshold {2.8};
    double m_minScale {2.5};
    double m_maxScale {4.5};
};