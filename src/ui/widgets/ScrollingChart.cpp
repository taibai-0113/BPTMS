#include "ScrollingChart.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <algorithm>
#include <cmath>

ScrollingChart::ScrollingChart(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(300, 150);
    // 让控件在布局里自动占满所有可用空间（水平 + 垂直都拉伸）
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 设置控件的背景角色，但实际背景颜色在 paintEvent 中手动绘制
    setBackgroundRole(QPalette::Base);
    // 控件背景是不透明的
    setAttribute(Qt::WA_OpaquePaintEvent);
}

// 设置标题
void ScrollingChart::setTitle(const QString &t) { m_title = t; update();}
// 设置显示窗口
void ScrollingChart::setTimeWindow(int s) { m_timeWindow = qMax(5, s); update();};
// 手动设置Y轴范围
void ScrollingChart::setYRange(double yMin, double yMax)
{
    m_yMin = yMin;
    m_yMax = yMax;
    m_autoY = false;
    update();
}

// 设置自动计算Y轴范围并更新图表
void ScrollingChart::setAutoYRange(bool on) { m_autoY = on; update();}

// 添加折线样式基本数据结构体
void ScrollingChart::addSeries(const QString &name, const QColor &color)
{
    if(!m_series.contains(name))
    {
        Series s;
        s.name = name;
        s.color = color;
        m_series.insert(name, s);
        m_seriesOrder.append(name);
    }
}

// 放置数据（放置数据前必须先添加样式结构体 ---> addSeries）
void ScrollingChart::pushValue(const QString &seriesName, double value)
{
    if(!m_series.contains(seriesName))return; // 如果没有这个系列，直接返回，不处理
    Series &s = m_series[seriesName]; // 拿到对应名字的数据系列引用
    s.values.push_back(value); // 在数据列表中添加数据（所以后面是连续的数据）
    if(s.values.size() > m_timeWindow)
        s.values.pop_front(); // 如果数据列表过长（超过X轴范围）就丢弃数据（因为看不见了）
    // 计算当前系列中所有存储数据的最小值和最大值，并记录下来
    s.yMin = *std::min_element(s.values.begin(), s.values.end());
    s.yMax = *std::max_element(s.values.begin(), s.values.end());
    update(); // 更新 调用 paintEvent(QPaintEvent *)
}

// 清除所有数据
void ScrollingChart::clearAll()
{
    for(auto &s : m_series)
    {
        s.values.clear();
        s.yMax = -1e9;
        s.yMin = 1e9;
    }
    update();
}

// 添加告警线
void ScrollingChart::addAlarmLine(double y, const QColor &c, const QString &lbl)
{
    m_alarmLines.append({y, c, lbl});
    update();
}

// 删除告警线
void ScrollingChart::removeAlarmLines() { m_alarmLines.clear(); update();}

QSize ScrollingChart::sizeHint() const {return{500, 200};}
QSize ScrollingChart::minimumSizeHint() const {return {300, 120};}

// ─── 绘制（update调用） ─────────────────────────────────────────────
void ScrollingChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // 抗锯齿，平滑过渡

    const QRect area = chartArea(); // 获取图表区域的x、y、宽、高

    // 填充图表区域 + ScrollingChart背景颜色
    p.fillRect(rect(), QColor(0x0d, 0x11, 0x17));
    // 填充图表区域颜色
    p.fillRect(area, QColor(0x16, 0x21, 0x3e));

    // 计算全局 Y 范围
    double yMin = m_yMin, yMax = m_yMax;
    if (m_autoY) {
        yMin = 1e9;
        yMax = -1e9;
        bool hasData = false;

        for (const auto &s : m_series) {
            if (s.values.empty()) continue;
            yMin = std::min(yMin, s.yMin);
            yMax = std::max(yMax, s.yMax);
            hasData = true;
        }

        if (!hasData) {
            // 没有任何数据，给一个安全默认范围
            yMin = 0.0;
            yMax = 1.0;
        } else {
            // 留 8% 边距
            double margin = (yMax - yMin) * 0.08;
            yMin -= margin;
            yMax += margin;

            // 如果范围过小（比如所有值相同），再加一点扩展
            if (yMax - yMin < 1e-6) {
                yMin -= 1.0;
                yMax += 1.0;
            }
        }
    }

    drawGrid(p, area, yMin, yMax);
    drawAlarmLines(p, area, yMin, yMax);
    drawSeries(p, area, yMin, yMax);
    drawLegend(p, area);

    // 标题
    if (!m_title.isEmpty())
    {
        p.setPen(QColor(0x88, 0xbb, 0xff));
        p.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
        p.drawText(QRect(0, 2, width(), MARGIN_TOP - 4),
                   Qt::AlignCenter, m_title);
    }

    // 边框
    p.setPen(QPen(QColor(0x30, 0x36, 0x3d), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(area);
}

void ScrollingChart::drawGrid(QPainter &p, const QRect &area,
                              double yMin, double yMax) const
{
    const int gridH = 5, gridV = 6;
    p.setPen(QPen(QColor(0x21, 0x26, 0x2e), 1, Qt::DashLine));
    p.setFont(QFont("Consolas", 8));

    // 水平网格线 + Y 轴刻度
    for (int i = 0; i <= gridH; ++i) {
        double val = yMin + (yMax - yMin) * i / gridH;
        // toPixelY ---> 将val转为像素
        int y   = static_cast<int>(toPixelY(val, area, yMin, yMax));
        // 绘画网格
        p.drawLine(area.left(), y, area.right(), y);
        p.setPen(QColor(0x55, 0x65, 0x80));
        // 绘画刻度数值
        p.drawText(QRect(0, y - 8, MARGIN_LEFT - 4, 16),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(val, 'f', 2));
        p.setPen(QPen(QColor(0x21, 0x26, 0x2e), 1, Qt::DashLine));
    }

    // 垂直网格线
    for (int i = 0; i <= gridV; ++i)
    {
        int x = area.left() + area.width() * i / gridV;
        // x的位置是变换的，然后从上往下拉线绘画网格
        p.drawLine(x, area.top(), x, area.bottom());
        p.setPen(QColor(0x55, 0x65, 0x80));
        int tSec = m_timeWindow - m_timeWindow * i / gridV;
        // 绘画刻度数值
        p.drawText(QRect(x - 15, area.bottom() + 2, 30, MARGIN_BOTTOM - 4),
                   Qt::AlignVCenter,
                   QStringLiteral("-%1s").arg(tSec));
        p.setPen(QPen(QColor(0x21, 0x26, 0x2e), 1, Qt::DashLine));
    }
}

// 绘画折线
void ScrollingChart::drawSeries(QPainter &p, const QRect &area,
                                double yMin, double yMax) const
{
    // // 遍历所有曲线，按添加顺序绘制
    for (const QString &name : m_seriesOrder) {
        const Series &s = m_series[name];
        if (s.values.size() < 2) continue; // 少于两个点无法形成折线

        int n       = static_cast<int>(s.values.size()); // 当前实际有多少个数据点
        int maxPts  = m_timeWindow; // 当前最多显示多少个点
        // 相邻两个点之间的横向距离；maxPts - 1是因为maxPts总共可以显示多少个点，总间隔需要-1
        float xStep   = static_cast<float>(area.width()) / (maxPts - 1);

        QPainterPath path; // 可以理解成 path 用来保存折线路径（当前已经记住的上一个点了）
        bool first = true;
        for (int i = 0; i < n; ++i) {
            // 数据移动核心，因为数据的加入，n变大了，所以maxPts - n + i变小了，所以导致是有点往左移了
            // * xStep代表把位置编号转换成像素距离
            int x = area.left() + static_cast<int>((maxPts - n + i) * xStep);
            // 值转像素
            double y = toPixelY(s.values[i], area, yMin, yMax);
            // 就是在生成整条折线路径
            if (first) { path.moveTo(x, y); first = false; } // 第一个点
            else        path.lineTo(x, y); // 后面的点连接成折线（从path记录的上一个点到xy）
        }

        // 复制折线路径，用于绘制折线下方的半透明填充区域
        QPainterPath fillPath = path;

        // 最新点所在的 X 坐标，也就是图表最右侧
        int lastX = area.left() + static_cast<int>((maxPts - 1) * xStep);

        // 最旧点所在的 X 坐标
        int firstX = area.left() + static_cast<int>((maxPts - n) * xStep);

        // 将折线路径向下闭合到图表底部，形成一个封闭区域
        fillPath.lineTo(lastX, area.bottom());
        fillPath.lineTo(firstX, area.bottom());
        fillPath.closeSubpath();

        // 使用曲线颜色的半透明版本填充折线下方区域
        QColor fillColor = s.color;
        fillColor.setAlpha(30);
        p.fillPath(fillPath, fillColor);

        // 绘制折线本身
        p.setPen(QPen(s.color, 1.5));
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);;
    }
}

// 绘画图例（每个折线的小展示）
void ScrollingChart::drawLegend(QPainter &p, const QRect &area) const
{
    const int itemW = 90, itemH = 16, pad = 6;
    int x = area.left() + pad;
    int y = area.top()  + pad;

    p.setFont(QFont("Microsoft YaHei", 8));
    for (const QString &name : m_seriesOrder) {
        const Series &s = m_series[name];
        p.fillRect(x, y + 5, 14, 3, s.color); // 颜色条
        p.setPen(QColor(0xcc, 0xcc, 0xcc));
        // 标题位置
        p.drawText(x + 18, y, itemW - 18, itemH,
                   Qt::AlignLeft | Qt::AlignVCenter, name);
        x += itemW;
        if (x + itemW > area.right() - pad) {
            x = area.left() + pad;
            y += itemH;
        }
    }
}

// 放置告警线
void ScrollingChart::drawAlarmLines(QPainter &p, const QRect &area,
                                    double yMin, double yMax) const
{
    p.setFont(QFont("Arial", 7));
    for (const auto &al : m_alarmLines) {
        if (al.y < yMin || al.y > yMax) continue;
        int y = static_cast<int>(toPixelY(al.y, area, yMin, yMax));
        p.setPen(QPen(al.color, 1, Qt::DashDotLine));
        p.drawLine(area.left(), y, area.right(), y);
        p.setPen(al.color);
        p.drawText(area.right() - 30, y - 10, al.label);
    }
}

// 图表区域的x、y、宽、高
QRect ScrollingChart::chartArea() const
{
    return QRect(MARGIN_LEFT, MARGIN_TOP,
                 width()  - MARGIN_LEFT - MARGIN_RIGHT,
                 height() - MARGIN_TOP  - MARGIN_BOTTOM);
}

// 将数值转为像素
double ScrollingChart::toPixelY(double val, const QRect &area,
                                double yMin, double yMax) const
{
    if (std::abs(yMax - yMin) < 1e-9) return area.center().y();
    double ratio = (val - yMin) / (yMax - yMin);
    return area.bottom() - ratio * area.height();
}

// 更新数据
void ScrollingChart::resizeEvent(QResizeEvent *) { update(); }