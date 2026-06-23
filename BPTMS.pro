QT += core gui widgets serialport sql

CONFIG += c++17
TARGET   = BPTMS
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += PROJECT_ROOT_DIR=\\\"$$PWD\\\"
CONFIG += debug_and_release
RESOURCES += resources/resources.qrc

INCLUDEPATH += src

SOURCES += \
    main.cpp \
    src/common/config/AppConfig.cpp \
    src/common/config/IniManager.cpp \
    src/common/config/JsonManager.cpp \
    src/common/logger/Logger.cpp \
    src/common/utils/ByteUtils.cpp \
    src/communication/CommWorker.cpp \
    src/communication/DbWorker.cpp \
    src/communication/SerialPortManager.cpp \
    src/core/AlarmChecker.cpp \
    src/core/device/BmsDevice.cpp \
    src/core/device/DeviceManager.cpp \
    src/core/device/DeviceSimulator.cpp \
    src/core/protocol/FrameParser.cpp \
    src/core/protocol/ModbusRTU.cpp \
    src/database/AlarmLogDao.cpp \
    src/database/DatabaseManager.cpp \
    src/database/SampleDataDao.cpp \
    src/database/TaskDao.cpp \
    src/ui/dialogs/AlarmConfigDialog.cpp \
    src/ui/dialogs/CreateTaskDialog.cpp \
    src/ui/dialogs/DeviceConfigDialog.cpp \
    src/ui/mainwindow/mainwindow.cpp \
    src/ui/panels/AlarmPanel.cpp \
    src/ui/panels/DevicePanel.cpp \
    src/ui/panels/HistoryPanel.cpp \
    src/ui/panels/LogPanel.cpp \
    src/ui/panels/MonitorPanel.cpp \
    src/ui/panels/TestPanel.cpp \
    src/ui/widgets/CellVoltageBar.cpp \
    src/ui/widgets/DataCard.cpp \
    src/ui/widgets/ScrollingChart.cpp

HEADERS += \
    src/common/config/AppConfig.h \
    src/common/config/IniManager.h \
    src/common/config/JsonManager.h \
    src/common/logger/Logger.h \
    src/common/utils/ByteUtils.h \
    src/communication/CommWorker.h \
    src/communication/DbWorker.h \
    src/communication/SerialPortManager.h \
    src/core/AlarmChecker.h \
    src/core/data/AlarmConfig.h \
    src/core/data/AlarmInfo.h \
    src/core/data/BatteryData.h \
    src/core/data/TestRecord.h \
    src/core/device/BmsDevice.h \
    src/core/device/DeviceManager.h \
    src/core/device/DeviceSimulator.h \
    src/core/device/IDevice.h \
    src/core/protocol/FrameParser.h \
    src/core/protocol/ModbusRTU.h \
    src/database/AlarmLogDao.h \
    src/database/DatabaseManager.h \
    src/database/SampleDataDao.h \
    src/database/TaskDao.h \
    src/ui/dialogs/AlarmConfigDialog.h \
    src/ui/dialogs/CreateTaskDialog.h \
    src/ui/dialogs/DeviceConfigDialog.h \
    src/ui/mainwindow/mainwindow.h \
    src/ui/panels/AlarmPanel.h \
    src/ui/panels/DevicePanel.h \
    src/ui/panels/HistoryPanel.h \
    src/ui/panels/LogPanel.h \
    src/ui/panels/MonitorPanel.h \
    src/ui/panels/TestPanel.h \
    src/ui/widgets/CellVoltageBar.h \
    src/ui/widgets/DataCard.h \
    src/ui/widgets/ScrollingChart.h

# FORMS += \
#     mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = resources/icon/ICON.ico