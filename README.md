# BPTMS
锂电池组测试与管理系统

<img src="https://img.shields.io/badge/version-v1.0.0-blue" width="96" title="" crop="0,0,1,1" id="pn7um" class="ne-image"><img src="https://img.shields.io/badge/license-MIT-green" width="78" title="" crop="0,0,1,1" id="KUmrd" class="ne-image"><img src="https://img.shields.io/badge/C%2B%2B-17-blueviolet" width="58" title="" crop="0,0,1,1" id="mhtMZ" class="ne-image"><img src="https://img.shields.io/badge/Qt-Widgets%20%7C%20SerialPort%20%7C%20SQL-brightgreen" width="178" title="" crop="0,0,1,1" id="z8AP2" class="ne-image"><img src="https://img.shields.io/badge/platform-Windows%20%7C%20Qt%20Desktop-lightgrey" width="190" title="" crop="0,0,1,1" id="ydnwd" class="ne-image"><img src="https://img.shields.io/badge/status-demo-orange" width="84" title="" crop="0,0,1,1" id="ITpKP" class="ne-image">

> BPTMS 是一个基于 Qt/C++ 的桌面端锂电池组测试与管理系统 Demo。项目提供设备连接、实时监控、测试任务、历史数据、告警记录、系统日志等功能，适合用于课程设计、桌面端 Qt 项目学习和小型开源项目展示。
>

---

## 目录
+ [项目简介](#项目简介)
+ [功能特性](#功能特性)
+ [技术栈](#技术栈)
+ [项目目录](#项目目录)
+ [快速开始](#快速开始)
+ [配置说明](#配置说明)
+ [环境变量示例](#环境变量示例)
+ [使用说明](#使用说明)
+ [开发说明](#开发说明)
+ [提交前检查](#提交前检查)
+ [贡献指南](#贡献指南)
+ [License](#license)

---

## 项目简介
BPTMS，全称 Battery Pack Testing & Management System，是一个面向锂电池组测试场景的桌面端管理工具。

它的目标不是做成完整商用系统，而是作为一个结构较完整、便于学习和展示的 Qt Demo 项目，帮助新手开发者理解：

+ Qt Widgets 桌面应用的基本组织方式
+ 设备通信、模拟器模式与 UI 展示的分层设计
+ SQLite 本地数据持久化
+ 测试任务、历史记录、告警记录和系统日志的基础管理流程
+ 如何把个人课程项目整理成可读、可运行、可维护的开源仓库

### 项目UI
<img src="https://cdn.nlark.com/yuque/0/2026/png/63448749/1782209291370-6012a7eb-5fc4-4f61-897c-8aba9b5fb4e5.png" width="2560" title="" crop="0,0,1,1" id="u8b3ec07f" class="ne-image">

### 项目思维导图
<img src="https://cdn.nlark.com/yuque/0/2026/png/63448749/1782209317672-9aa7cf44-927e-4380-90f0-6deb24a17782.png" width="12288" title="" crop="0,0,1,1" id="u851fc457" class="ne-image">

---

## 功能特性
### 设备连接
+ 支持模拟器模式，便于在没有真实硬件时运行 Demo
+ 支持真实串口设备连接
+ 支持串口号、波特率、从站 ID、采样间隔等基础参数配置
+ 显示连接状态、运行时长、帧计数和通信质量

### 实时监控
+ 展示电池组总电压、总电流、SOC、SOH、温度、功率和循环次数等运行指标
+ 提供实时曲线视图
+ 提供单体电压条形图
+ 支持根据告警阈值进行状态提示

### 测试任务
+ 支持创建测试任务
+ 支持启动、暂停、继续和停止任务
+ 支持查看历史任务列表
+ 运行中的采样数据可关联到对应任务

### 历史数据
+ 支持按任务查询历史采样数据
+ 支持表格化展示历史记录
+ 支持导出 CSV，便于后续分析或报告整理

### 告警记录
+ 支持告警记录展示
+ 支持确认单条告警
+ 支持一键确认全部告警
+ 支持按任务加载告警记录

### 系统日志
+ 支持不同级别日志显示
+ 支持日志自动滚动
+ 支持清空和保存日志内容
+ 便于调试通信、数据库和任务流程

---

## 技术栈
| 分类 | 技术 |
| --- | --- |
| 编程语言 | C++17 |
| GUI 框架 | Qt Widgets |
| 通信相关 | Qt SerialPort、Modbus RTU 风格通信封装 |
| 数据库 | SQLite、Qt SQL |
| 配置管理 | INI、JSON |
| 日志 | 本地日志文件 + GUI 日志面板 |
| 构建方式 | Qt Creator / qmake 项目 |
| 运行平台 | Windows 桌面端优先，其他 Qt Desktop 平台可自行适配 |


---

## 项目目录
推荐目录结构如下：

```latex
BPTMS/
├── BPTMS.pro                    # qmake 工程文件
├── main.cpp                     # 程序入口
├── README.md                    # 项目说明
│
├── docs/                        # 项目文档
│   ├── BPTMS_思维导图图片.png   # 思维导图图片
│   ├── BPTMS思维导图.xmind      # 思维导图
│   └── UI.png                   # BPTMS界面
│
├── resources/                   # Qt资源文件
│   ├── resources.qrc
│   ├── icons/                   # 图标资源
│   └── styles/
│       └── dark.qss             # 主题样式表
│
├── config/                      # 默认配置文件
│   ├── app.ini                  # 应用默认配置
│   └── alarm_config.json        # 默认告警阈值
│
├── logs/                        # 日志文件
│
└── src/                         # 源代码根目录
    │
    ├── common/                  # 通用基础层
    │   │
    │   ├── logger/
    │   │   ├── Logger.h         # 日志类声明
    │   │   └── Logger.cpp
    │   │
    │   ├── config/
    │   │   ├── AppConfig.h      # 全局配置管理
    │   │   ├── AppConfig.cpp
    │   │   ├── IniManager.h     # INI读写封装
    │   │   ├── IniManager.cpp
    │   │   ├── JsonManager.h    # JSON读写封装
    │   │   └── JsonManager.cpp
    │   │
    │   └── utils/
    │       ├── ByteUtils.h      # 字节操作工具（CRC16等）
    │       └── ByteUtils.cpp
    │
    ├── core/                    # 核心业务层
    │   │
    │   ├── AlarmChecker.h       # 告警检测实现
    │   ├── AlarmChecker.cpp
    │   │
    │   ├── data/                # 数据模型（纯数据结构）
    │   │   ├── BatteryData.h    # 实时采样数据结构体
    │   │   ├── TestRecord.h     # 测试任务/记录结构体
    │   │   ├── AlarmInfo.h      # 告警信息结构体
    │   │   └── AlarmConfig.h    # 告警阈值配置结构体
    │   │
    │   ├── protocol/            # 协议解析层
    │   │   ├── ModbusRTU.h      # Modbus RTU 帧构造与解析
    │   │   ├── ModbusRTU.cpp
    │   │   ├── FrameParser.h    # 通用数据帧解析器
    │   │   └── FrameParser.cpp
    │   │
    │   └── device/              # 设备抽象层
    │       ├── IDevice.h        # 设备接口（纯虚类）
    │       ├── BmsDevice.h      # 真实BMS设备实现
    │       ├── BmsDevice.cpp
    │       ├── DeviceSimulator.h  # 软件模拟器（产生仿真数据）
    │       ├── DeviceSimulator.cpp
    │       ├── DeviceManager.h  # 设备管理器（unique_ptr管理设备，已经弃用）
    │       └── DeviceManager.cpp 
    │
    ├── communication/           # 通信层
    │   ├── SerialPortManager.h  # 串口管理（QSerialPort封装）
    │   ├── SerialPortManager.cpp
    │   ├── CommWorker.h         # 通信Worker（运行于子线程）
    │   ├── CommWorker.cpp
    │   ├── DbWorker.h           # 数据库Worker（运行于子线程）
    │   └── DbWorker.cpp
    │
    ├── database/                # 数据持久层
    │   ├── DatabaseManager.h    # 数据库连接管理（单例）
    │   ├── DatabaseManager.cpp
    │   ├── TaskDao.h            # 测试记录 DAO
    │   ├── TaskDao.cpp
    │   ├── SampleDataDao.h      # 采样数据 DAO
    │   ├── SampleDataDao.cpp
    │   ├── AlarmLogDao.h        # 告警日志 DAO
    │   └── AlarmLogDao.cpp
    │
    └── ui/                      # 界面层
        │
        ├── mainwindow/
        │   ├── MainWindow.h
        │   └── MainWindow.cpp
        │
        ├── panels/              # 主区域功能面板（QWidget子类）
        │   ├── DevicePanel.h    # 左侧设备连接面板
        │   ├── DevicePanel.cpp
        │   ├── MonitorPanel.h   # 实时监控面板
        │   ├── MonitorPanel.cpp
        │   ├── TestPanel.h      # 测试任务管理面板
        │   ├── TestPanel.cpp
        │   ├── HistoryPanel.h   # 历史数据面板
        │   ├── HistoryPanel.cpp
        │   ├── AlarmPanel.h     # 告警记录面板
        │   ├── AlarmPanel.cpp
        │   ├── LogPanel.h       # 日志查看面板
        │   └── LogPanel.cpp           
        │
        ├── dialogs/             # 对话框
        │   ├── AlarmConfigDialog.h    # 告警阈值配置对话框
        │   ├── AlarmConfigDialog.cpp    
        │   ├── CreateTaskDialog.h     # 任务创建配置对话框
        │   ├── CreateTaskDialog.cpp    
        │   ├── DeviceConfigDialog.h   # 设备/串口配置对话框
        │   └── DeviceConfigDialog.cpp
        │
        └── widgets/             # 自定义控件
            ├── DataCard.h             # 自定义数据卡片
            ├── DataCard.cpp
            ├── CellVoltageBar.h       # 单体电压柱状图
            ├── CellVoltageBar.cpp
            ├── ScrollingChart.h       # 实时曲线图控件
            └── ScrollingChart.cpp
```

---

## 快速开始
### 1. 克隆项目
```bash
git clone https://github.com/taibai-0113/BPTMS.git
cd BPTMS
```

### 2. 准备开发环境
建议安装：

+ Qt 5.15.x 或 Qt 6.x
+ Qt Creator
+ 支持 C++17 的编译器
    - Windows：MinGW / MSVC
    - Linux：GCC / Clang
    - macOS：Clang
+ Qt 模块：
    - Widgets
    - SerialPort
    - SQL
+ SQLite 驱动

### 3. 使用 Qt Creator 运行
1. 打开 Qt Creator
2. 选择 `File` → `Open File or Project`
3. 打开项目根目录下的 `BPTMS.pro`
4. 选择合适的 Desktop Kit
5. 点击 `Build`
6. 点击 `Run`

首次运行建议勾选“使用模拟器”，这样无需真实串口设备也可以查看主要界面和数据流转效果。

### 4. 使用命令行构建
如果你已经配置好 Qt 命令行环境，可以尝试：

```bash
qmake BPTMS.pro
make
```

在 Windows + MinGW 环境中可能需要使用：

```bash
qmake BPTMS.pro
mingw32-make
```

如果使用 MSVC 环境，可能需要使用：

```bash
qmake BPTMS.pro
nmake
```

---

## 配置说明
项目运行时会使用本地配置文件保存常用设置。推荐保留示例配置，但不要把个人真实设备参数、日志和数据库文件提交到公开仓库。

### 应用配置
建议在项目根目录下使用：

```latex
config/app.ini
```

示例：

```properties
[Serial]
port=COM1
baudRate=9600
slaveId=1
simulatorMode=false

[Polling]
interval=1000

[Chart]
timeWindow=60
```

### 告警配置
建议在项目根目录下使用：

```latex
config/alarm_config.json
```

示例结构：

```json
{
    "cellOverVolt": 3.6,
    "cellUnderVolt": 2.8,
    "overCurrent": 30,
    "overTemp": 55,
    "overVoltage": 28.5,
    "underVoltage": 22.4
}
```

以上数值仅用于本地 Demo 演示。真实设备或实验环境中，请根据硬件参数和安全要求重新设置。

---

## 环境变量示例
当前项目主要通过本地 INI/JSON 配置文件管理参数。为了方便后续 CI、打包或跨平台部署，可以在仓库中提供一个 `.env.example` 文件作为约定示例：

```bash
# .env.example

# Application
BPTMS_APP_ENV=development
BPTMS_CONFIG_DIR=./config
BPTMS_LOG_DIR=./logs
BPTMS_DB_PATH=./app.db

# Device
BPTMS_SIMULATOR_MODE=true
BPTMS_SERIAL_PORT=COM3
BPTMS_SERIAL_BAUD_RATE=9600
BPTMS_SLAVE_ID=1

# Polling
BPTMS_POLLING_INTERVAL_MS=1000
BPTMS_CHART_TIME_WINDOW_SEC=60
```

---

## 使用说明
### 模拟器模式
适合以下情况：

+ 没有真实电池设备
+ 想快速查看 UI 效果
+ 想演示任务、历史数据、告警和日志流程
+ 想录制课程项目 Demo 视频

操作流程：

1. 打开程序
2. 在左侧设备控制面板中勾选“使用模拟器”
3. 设置采样间隔
4. 点击“连接”
5. 进入“实时监控”查看数据变化
6. 在“测试任务”中创建并启动任务（可选）
7. 停止任务后，可在“历史数据”中查看采样记录

### 真实串口模式
适合已经连接真实下位机或 BMS 设备的情况。

操作流程：

1. 取消勾选“使用模拟器”
2. 点击“刷新”加载本机串口
3. 选择串口号和波特率
4. 设置从站 ID
5. 点击“连接”
6. 在“测试任务”中创建并启动任务（可选）
7. 停止任务后，可在“历史数据”中查看采样记录

使用真实硬件时，请先确认设备供电、串口连接、通信参数和安全阈值配置正确。

---

## 开发说明
### 模块职责概览
| 模块 | 职责 |
| --- | --- |
| `main.cpp` | 应用启动入口 |
| `ui/mainwindow/` | 主窗口、菜单栏、工具栏、状态栏和页面组织 |
| `ui/panels/` | 设备、监控、任务、历史、告警和日志面板 |
| `ui/dialogs/` | 设备配置、告警配置和任务创建弹窗 |
| `ui/widgets/` | 数据卡片、曲线图和电压条等自定义控件 |
| `communication/` | 串口通信与后台通信工作流 |
| `core/device/` | 设备抽象、模拟设备和真实设备 |
| `core/protocol/` | 通信协议封装与数据转换入口 |
| `core/data/` | 业务数据结构 |
| `database/` | 数据库初始化、任务数据、采样数据和告警数据访问 |
| `common/config/` | INI/JSON 配置读写 |
| `common/logger/` | 日志输出与日志文件管理 |
| `common/utils/` | 通用字节处理工具 |


### 推荐开发流程
```bash
git checkout -b feature/your-feature-name
```

开发完成后：

```bash
git status
git add .
git commit -m "feat: add your feature"
git push origin feature/your-feature-name
```

然后在 GitHub 上发起 Pull Request。

### 建议的提交信息格式
推荐使用简化版 Conventional Commits：

```latex
feat: 新增功能
fix: 修复问题
docs: 修改文档
style: 调整格式或样式
refactor: 重构代码
test: 增加测试
chore: 构建、配置或杂项调整
```

示例：

```latex
docs: improve quick start guide
fix: handle serial connection failure
feat: add csv export guide
```

---

## 提交前检查
公开仓库前，建议确认以下内容：

- [ ] `README.md` 已放在仓库根目录
- [ ] 已添加 `LICENSE`
- [ ] 已添加 `.gitignore`
- [ ] 未提交 `build/`、`debug/`、`release/` 等构建目录
- [ ] 未提交本机真实 `app.db`
- [ ] 未提交个人真实串口配置
- [ ] 未提交包含个人路径、账号、设备编号的日志
- [ ] 项目可以通过 Qt Creator 正常打开
- [ ] 默认模拟器模式可以运行
- [ ] README 中的 GitHub 用户名、仓库地址和截图路径已替换

推荐 `.gitignore` 示例：

```plain
# Qt Creator
*.user
*.pro.user
*.autosave

# Build output
build/
build-*/
debug/
release/
*.o
*.obj
*.exe
*.dll
*.so
*.dylib

# Runtime data
app.db
*.db
*.sqlite
logs/
config/app.ini
config/alarm_config.json

# System files
.DS_Store
Thumbs.db
```

---

## 贡献指南
欢迎提交 Issue 或 Pull Request 来改进项目。

你可以参与：

+ 修复界面显示问题
+ 改进 README、注释或使用说明
+ 增加截图、演示视频和部署说明
+ 优化配置体验
+ 改进跨平台构建说明
+ 增加测试或示例数据

贡献步骤：

1. Fork 本仓库
2. 创建功能分支
3. 完成修改并本地测试
4. 提交清晰的 commit message
5. 发起 Pull Request
6. 在 PR 描述中说明修改原因、影响范围和测试方式

请注意：

+ 不要在 Issue 或 PR 中提交真实设备敏感数据
+ 不要上传个人日志、真实采集数据库或隐私路径
+ 不要直接粘贴大段核心源代码到公开讨论区
+ 如果涉及通信协议或硬件参数，请尽量用概括性描述说明问题

---

## License
```latex
MIT License

Copyright (c) 2026 BPTMS Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

请以仓库根目录中的 `LICENSE` 文件为准。

---

## 致谢
<font style="color:rgb(15, 17, 21);">这是我个人第二次开源的小型演示项目。感谢每一位愿意阅读、反馈和参与改进的开发者。</font>

<font style="color:rgb(15, 17, 21);">本项目的大纲由 Claude-Opus-4.8-thinking 辅助搭建，具体实现由我独立完成。如果项目中存在错误或不准确之处，给你的学习或开发带来了不便，敬请谅解。</font>

<font style="color:rgb(15, 17, 21);">如果这个项目对你学习 Qt、C++ 上位机开发或整理课程设计有所帮助，欢迎点个 Star！</font>
