#include "CellVoltageBar.h"
#include <QPainter>
#include <QPainterPath>

CellVoltageBar::CellVoltageBar(QWidget *parent) : QWidget(parent)
{
    // m_voltages.fill(3.7);
    m_voltages = {0, 0, 0, 0, 0, 0, 0, 0};
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(120);
}

// 放置8节电池数据
void CellVoltageBar::setVoltages(const std::array<double, 8> &v)
{
    m_voltages = v;
    update();
}

// 放置过压低压阈值
void CellVoltageBar::setThresholds(double over, double under)
{
    m_overThreshold = over;
    m_underThreshold = under;
    update();
}

QSize CellVoltageBar::sizeHint() const {return {400, 130};}
QSize CellVoltageBar::minimumSizeHint() const {return {200, 100};}

void CellVoltageBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);;
    p.setRenderHint(QPainter::Antialiasing);

    const int W = width();          // 控件的总宽度
    const int H = height();         // 总高度
    const int margin = 10;          // 上下左右边距
    const int labelH = 18;          // 底部文字区域高度
    const int barArea = H - labelH - margin * 2;  // 柱子可用的高度
    const int n = 8;                // 8 节电池
    const int gap = 4;              // 柱子之间的间隙
    const int barW = (W - margin * 2 - gap * (n - 1)) / n; // 每个柱子的宽度

    // 背景
    p.fillRect(rect(), QColor(0x16, 0x21, 0x3e));

    for(int i = 0; i < n; ++i)
    {
        double v = m_voltages[i];
        // norm是当前数据在整个画布中的一个比例
        double norm = (v - m_minScale) / (m_maxScale - m_minScale);
        norm = qBound(0.0, norm, 1.0);

        int barH = static_cast<int>(norm * barArea);
        int x = margin + i * (barW + gap);
        int y = margin + barArea - barH;

        // 确定颜色
        QColor barColor;
        bool alarm = false;
        if (v > m_overThreshold) {
            barColor = QColor(0xe7, 0x4c, 0x3c);  // 红：过压
            alarm = true;
        } else if (v < m_underThreshold) {
            barColor = QColor(0xe6, 0x7e, 0x22);  // 橙：欠压
            alarm = true;
        } else {
            barColor = QColor(0x27, 0xae, 0x60);  // 绿：正常
        }

        // 背景槽
        p.setBrush(QColor(0x0f, 0x34, 0x60));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(x, margin, barW, barArea, 3, 3);

        // 当前值条
        if(barH > 0)
        {
            p.setBrush(barColor);
            p.drawRoundedRect(x, y, barW, barH, 3, 3);
        }

        // 告警边框
        if(alarm)
        {
            p.setPen(QPen(barColor, 1.5));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(x, margin, barW, barArea, 3, 3);
        }

        // 数值文字
        p.setPen(alarm ? QColor(0xe7, 0x4c, 0x3c) : Qt::white);
        p.setFont(QFont("Arial", 8));
        p.drawText(QRect(x, H - labelH - margin + 4, barW, labelH),
                   Qt::AlignCenter,
                   QString::number(v, 'f', 3));

        // 编号
        p.setPen(QColor(0x88, 0x88, 0x88));
        p.setFont(QFont("Arial", 7));
        p.drawText(QRect(x, H - 12, barW, 14),
                   Qt::AlignCenter,
                   QStringLiteral("C%1").arg(i + 1));
    }

    // 阈值基准线
    auto drawThLine = [&](double val, QColor color, const QString &label) {
        double norm = (val - m_minScale) / (m_maxScale - m_minScale);
        int    ly   = static_cast<int>(margin + barArea * (1.0 - norm));
        p.setPen(QPen(color, 1, Qt::DashLine));
        // p.drawLine(margin, ly, W - margin, ly);
        // 微调后
        p.drawLine(margin + 1, ly, W - margin - 7, ly);
        p.setPen(color);
        p.setFont(QFont("Arial", 7));
        p.drawText(W - margin - 30, ly - 2, label);
    };
    drawThLine(m_overThreshold,  QColor(0xFF, 0x6B, 0x6B), "OV");
    drawThLine(m_underThreshold, QColor(0xFF, 0xB3, 0x47), "UV");
}