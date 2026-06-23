#pragma once

#include <QWidget>
#include <QDebug>
#include <QMap>
#include <QColor>
#include <QPair>

class QPainter;

// ─── 轻量级滚动折线图（不依赖 QtCharts）─────────────────
// 支持多条曲线、自动滚动时间窗口、Y轴自适应、告警线
class ScrollingChart : public QWidget
{
    Q_OBJECT
public:
    // 折线样式基本数据结构体
    struct Series{
        QString name;
        QColor color;
        QList<double> values;
        double yMin {1e9};
        double yMax {-1e9};
    };

    explicit ScrollingChart(QWidget *parent = nullptr);

    void setTitle (const QString &title); // 放置标题
    void setTimeWindow (int seconds); // 显示最近 N 秒的数据
    void setYRange (double yMin, double yMax); // 放置Y轴范围
    void setAutoYRange (bool on); // 是否自动计算Y轴
    void addSeries (const QString &name, const QColor &color); // 添加Series
    void pushValue (const QString &seriesName, double value); // 放置数值
    void clearAll (); // 清除所有数据
    void addAlarmLine(double y, const QColor &color, const QString &label); // 添加告警线
    void removeAlarmLines(); //  删除告警线

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    // Qt 在需要重绘控件时自动调用，update() 会请求触发该函数
    void paintEvent (QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // 绘画各种数据
    void drawGrid (QPainter &p, const QRect &area,
                  double yMin, double yMax)const;
    void drawSeries (QPainter &p, const QRect &area,
                    double yMin, double yMax) const;
    void drawLegend (QPainter &p, const QRect &area) const;
    void drawAlarmLines (QPainter &p, const QRect &area,
                        double yMin, double yMax) const;
    // 返回图标区域的x、y、宽、高
    QRect chartArea() const;
    // 数值 ---> 像素
    double toPixelY (double val, const QRect &area,
                    double yMin, double yMax) const;

    QString m_title; // 图表标题
    int m_timeWindow {60}; // 显示窗口，相当于X轴
    double m_yMin {0.0}; // Y轴最大值
    double m_yMax {100.0}; // Y轴最小值
    bool m_autoY {false};

    QMap<QString, Series> m_series;
    QList<QString> m_seriesOrder;

    struct AlarmLine {double y; QColor color; QString label;};
    QList<AlarmLine> m_alarmLines;

    // 上左下右的margin间隙
    static const int MARGIN_LEFT = 55;
    static const int MARGIN_RIGHT = 12;
    static const int MARGIN_TOP = 30;
    static const int MARGIN_BOTTOM = 30;
};