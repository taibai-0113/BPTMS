#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>

#include "common/logger/Logger.h"
#include "core/data/BatteryData.h"
#include "core/data/AlarmInfo.h"
#include "ui/mainwindow/MainWindow.h"

int main(int argc, char *argv[])
{
    // 开启整体高 DPI 缩放适配
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // 开启高清位图适配
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
    app.setApplicationName   ("BPTMS");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName  ("BPTMS Project");

    // ── 注册跨线程信号元类型 ──────────────────────────
    qRegisterMetaType<BatteryData>("BatteryData");
    qRegisterMetaType<AlarmInfo>  ("AlarmInfo");
    qRegisterMetaType<LogLevel>   ("LogLevel");
    qRegisterMetaType<QList<TestRecord>>("QList<TestRecord>");

    // ── 启动日志 ──────────────────────────────────────
    Logger::instance()->setMinLevel(LogLevel::DEBUG);
    LOG_I("========================================");
    LOG_I(QStringLiteral("BPTMS v1.0 启动  Qt=%1  Thread=0x%2")
              .arg(QT_VERSION_STR)
              .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 16));
    LOG_I("========================================");

    // ── 启动画面（可选）──────────────────────────────
    QPixmap splashPx(400, 200);
    splashPx.fill(QColor(0x16, 0x21, 0x3e));
    QSplashScreen splash(splashPx);
    splash.showMessage(
        "<span style='color:#3498db;font-size:16px;'>"
        "<b>BPTMS</b> · 正在初始化...</span>",
        Qt::AlignCenter,
        Qt::white);
    splash.show();
    app.processEvents();

    // ── 主窗口 ────────────────────────────────────────
    MainWindow w;
    w.show();
    splash.finish(&w);

    LOG_I("主窗口已显示，进入事件循环");
    int ret = app.exec();

    LOG_I(QStringLiteral("程序退出，代码=%1").arg(ret));
    return ret;
}