# TB6612FNG 双路直流电机 + 舵机驱动板

基于 TI MSPM0G3507 的电机驱动板固件，通过 UART 二进制协议控制两路 TB6612FNG 直流电机和一路 PWM 舵机。

本工程由 [TI-MSPM0-Template](https://github.com/Tommy20240110/TI-MSPM0-Template) 生成，集成了：

- TI SysConfig 外设配置与代码生成
- Arm GNU Toolchain（`arm-none-eabi-gcc`）
- CMake + Ninja 构建系统
- VS Code IntelliSense、构建任务与 Cortex-Debug 调试
- OpenOCD 烧录与调试
- DAPLink、J-Link 和 XDS110 调试器配置

## 硬件

- **MCU**: MSPM0G3507
- **电机驱动**: TB6612FNG（双路）
- **编码器**: 增量编码器（可选）
- **舵机**: PWM 舵机
- **通信**: UART（115200 8N1），二进制协议

## 固件架构

```text
Application/         应用层 — UART 协议解析、命令分发
Board/               板级层 — 电机初始化、资源实例
Device/
├─ encoder/          编码器抽象 + 增量编码器驱动
├─ interface/        TB6612FNG 芯片驱动
└─ motor/            电机抽象 + BDC / 舵机 / TB6612FNG 驱动
Platform/            硬件抽象层 (HAL)
├─ arch/MSPM0/       MSPM0 架构实现
├─ communications/   UART / I2C / SPI / CAN
├─ gpio/             GPIO
├─ timers/           PWM / 定时器 / 输入捕获
├─ tests/            单元测试 + Fake 实现
└─ examples/         驱动示例
Utils/               工具宏 (container_of, Q16.16 定点数)
```

## 工作流程

```text
configure.bat
    ↓
生成本机配置、CMake Preset、工具链和 VS Code 配置
    ↓
syscfg.bat
    ↓
SysConfig 生成外设代码、链接脚本和库依赖
    ↓
CMake + Ninja
    ↓
生成 ELF、HEX 和 MAP
    ↓
OpenOCD + 调试器
    ↓
烧录并进入源码调试
```

## 环境要求

本工程主要面向 Windows 环境。

| 工具 | 作用 |
| --- | --- |
| TI SysConfig | 生成外设初始化代码和链接配置 |
| TI MSPM0 SDK | 提供设备头文件、DriverLib 和启动文件 |
| Arm GNU Toolchain | 编译、链接和 GDB 调试 |
| CMake 3.28 或更高版本 | 生成构建系统 |
| Ninja | 执行构建 |
| OpenOCD | 烧录并提供 GDB Server |
| VS Code（可选） | 编辑、构建和图形化调试 |

### 已验证的调试器

| 调试器 | OpenOCD 烧录 | 说明 |
| --- | --- | --- |
| DAPLink / CMSIS-DAP | 可用 | 可直接通过 SWD 正常烧录 |
| XDS110 | 可用 | 可直接通过 OpenOCD 正常烧录 |
| J-Link | 可用，但需调整驱动 | 使用 OpenOCD 前通常需要将 J-Link USB 接口更换为 WinUSB 或 libusb 兼容驱动 |

## 目录结构

```text
.
├─ .cmake/
│  ├─ template/                  CMake 配置模板
│  ├─ toolchain/                 生成的本机工具链文件
│  └─ utils/                     SysConfig 库解析工具
├─ .ti/
│  ├─ DriverBoard_TB6612FNG.syscfg  SysConfig 工程
│  ├─ syscfg.bat                 SysConfig 代码生成脚本
│  └─ generate/                  SysConfig 中间生成文件
├─ .vscode/
│  ├─ template/                  VS Code 配置模板
│  ├─ openocd_daplink.cfg        DAPLink 配置
│  ├─ openocd_jlink.cfg          J-Link 配置
│  └─ openocd_xds110.cfg         XDS110 配置
├─ Core/
│  ├─ Inc/                       SysConfig 生成的头文件
│  ├─ Src/                       SysConfig 生成的源文件
│  └─ Startup/                   芯片的 GCC 启动文件
├─ User/                         用户应用代码
│  ├─ Application/               UART 协议 + main
│  ├─ Board/                     板级初始化
│  ├─ Device/                    电机 / 编码器驱动
│  ├─ Platform/                  硬件抽象层
│  └─ Utils/                     工具宏
├─ scripts/
│  ├─ configure.bat              首次配置脚本
│  └─ mspm0_chip_db.csv          芯片参数数据库
├─ CMakeLists.txt
├─ CMakePresets.json             生成的项目级 Preset
└─ CMakeUserPresets.json         生成的本机路径 Preset
```

## 快速开始

### 1. 运行配置脚本

在工程根目录打开 PowerShell：

```powershell
.\scripts\configure.bat
```

首次运行时选择重新配置，依次输入本机安装路径：

```text
TI Sysconfig path:              D:\Tools\TI\Sysconfig
TI MSPM0 SDK path:              D:\Tools\TI\mspm0_sdk_2_10_00_04
GCC path:                       D:\Tools\ArmGNU
OpenOCD path:                   D:\Tools\OpenOCD
Enter chip model:               MSPM0G3507
```

### 2. 运行 SysConfig

```powershell
.\.ti\syscfg.bat
```

### 3. 配置 CMake

```powershell
cmake --preset user-debug
```

### 4. 编译工程

```powershell
cmake --build build
```

构建产物位于 `build/outputs/`。

## VS Code 一键构建

完成首次配置后，在 VS Code 中按 `Ctrl + Shift + B`，默认的 `CMake build` 任务会依次执行：

```text
TI Sysconfig Prebuild → CMake configure → CMake build
```

也可以通过"终端 → 运行任务"执行 Flash / Debug 任务。

## 烧录与调试

连接调试器与目标板（SWDIO、SWCLK、GND），确认目标板供电正常。

在 VS Code 中运行对应任务：
- `Flash (DAPLink)` / `Flash (XDS110)` / `Flash (J-Link)`
- `Debug (DAPLink)` — 启动 OpenOCD + GDB，停在 `main`

## UART 通信协议

协议格式详见 [User/Application/uart_protocol.h](User/Application/uart_protocol.h)。

支持命令：
- `SET_MOTOR` — 设置电机方向与速度（左/右/双侧）
- `SET_SERVO` — 设置舵机角度
- `EMERGENCY_STOP` — 急停（双侧电机 coast + 舵机回中）

## 常见问题

### `config.ini not found`

尚未完成首次配置。运行 `.\scripts\configure.bat`。

### CMake 找不到 DriverLib

确认 TI SDK 路径正确，并先运行 `.\ti\syscfg.bat`。

### OpenOCD 找不到调试器

检查 USB 连接、驱动、接线和供电。J-Link 可能需要更换 WinUSB/libusb 驱动。

## 已验证配置

- MSPM0G3507
- TI MSPM0 SDK 2.10.00.04
- Arm GNU Toolchain 15.2
- CMake + Ninja
- OpenOCD
- DAPLink / XDS110：烧录和调试通过
