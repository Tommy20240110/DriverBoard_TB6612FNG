# TI MSPM0 通用工程模板

这是一个面向 TI MSPM0 系列 MCU 的通用嵌入式工程模板，集成了：

- TI SysConfig 外设配置与代码生成
- Arm GNU Toolchain（`arm-none-eabi-gcc`）
- CMake + Ninja 构建系统
- VS Code IntelliSense、构建任务与 Cortex-Debug 调试
- OpenOCD 烧录与调试
- DAPLink、J-Link 和 XDS110 调试器配置
- 根据芯片型号自动选择架构、启动文件和 DriverLib

首次克隆工程后，只需运行一次配置脚本并填写本机工具路径，即可完成代码生成、编译、烧录和调试。

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

本模板主要面向 Windows 环境。

工程采用以下开发与调试环境：

```text
TI SysConfig + CMake + Ninja + GCC + OpenOCD
```

各工具的职责如下：

| 工具 | 作用 |
| --- | --- |
| TI SysConfig | 生成外设初始化代码和链接配置 |
| TI MSPM0 SDK | 提供设备头文件、DriverLib 和启动文件 |
| Arm GNU Toolchain | 编译、链接和 GDB 调试 |
| CMake 3.28 或更高版本 | 生成构建系统 |
| Ninja | 执行构建 |
| OpenOCD | 烧录并提供 GDB Server |
| VS Code（可选） | 编辑、构建和图形化调试 |

目前已验证的调试器兼容性：

| 调试器 | OpenOCD 烧录 | 说明 |
| --- | --- | --- |
| DAPLink / CMSIS-DAP | 可用 | 可直接通过 SWD 正常烧录 |
| XDS110 | 可用 | 可直接通过 OpenOCD 正常烧录 |
| J-Link | 可用，但需调整驱动 | 使用 OpenOCD 前通常需要将 J-Link USB 接口更换为 WinUSB 或 libusb 兼容驱动 |

如果使用 VS Code，建议安装：

- C/C++
- CMake Tools
- Cortex-Debug

安装后可在终端检查基础工具：

```powershell
cmake --version
ninja --version
```

Arm GCC 和 OpenOCD 不要求加入系统 `PATH`，配置脚本会记录它们的安装路径。

## 目录结构

```text
.
├─ .cmake/
│  ├─ template/                  CMake 配置模板
│  ├─ toolchain/                 生成的本机工具链文件
│  └─ utils/                     SysConfig 库解析工具
├─ .ti/
│  ├─ TI-MSPM0-Template.syscfg   SysConfig 工程
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
│  └─ Startup/                   对应芯片的 GCC 启动文件
├─ User/                         用户应用代码
├─ scripts/
│  ├─ configure.bat              首次配置脚本
│  ├─ config.ini                 生成的本机配置
│  └─ mspm0_chip_db.csv          芯片参数数据库
├─ CMakeLists.txt
├─ CMakePresets.json             生成的项目级 Preset
└─ CMakeUserPresets.json         生成的本机路径 Preset
```

## 快速开始

下面的流程假设工程已经从 Git 克隆到本地。

### 1. 运行配置脚本

可以双击：

```text
scripts\configure.bat
```

也可以在工程根目录打开 PowerShell：

```powershell
.\scripts\configure.bat
```

首次运行时选择重新配置：

```text
Do you want to reconfigure? (Y/N): Y
```

随后依次输入本机安装路径：

```text
TI Sysconfig path:              D:\Tools\TI\Sysconfig
TI MSPM0 SDK path:              D:\Tools\TI\mspm0_sdk_2_10_00_04
GCC path:                       D:\Tools\ArmGNU
OpenOCD path:                   D:\Tools\OpenOCD
Enter chip model:               MSPM0G3507
```

路径既可以使用 `\`，也可以使用 `/`。脚本会自动将路径转换为工程配置所需的格式。

芯片型号不区分大小写，但必须存在于
[`scripts/mspm0_chip_db.csv`](scripts/mspm0_chip_db.csv) 中。例如：

```text
MSPM0G3507
mspm0g3507
```

如果输入了不存在的型号，脚本会停止并提示：

```text
ERROR: Chip model "..." not found in database!
```

重新运行脚本并输入正确型号即可。

脚本识别芯片后会自动完成：

1. 生成 `CMakePresets.json`
2. 生成 `CMakeUserPresets.json`
3. 生成 `.cmake/toolchain/toolchain.cmake`
4. 生成 `scripts/config.ini`
5. 从 TI SDK 复制对应芯片的 GCC 启动文件

当脚本询问是否生成 VS Code 配置时，推荐输入：

```text
Do you want to generate VS Code configuration? (Y/N): Y
```

随后会生成：

- `.vscode/c_cpp_properties.json`
- `.vscode/launch.json`
- `.vscode/tasks.json`

这些文件包含本机绝对路径，因此默认不会提交到 Git。

### 2. 运行 SysConfig

在工程根目录执行：

```powershell
.\.ti\syscfg.bat
```

脚本会调用 TI SysConfig CLI，并将主要文件整理到以下位置：

```text
Core/Src/ti_msp_dl_config.c
Core/Inc/ti_msp_dl_config.h
.ti/generate/device_linker.lds
.ti/generate/device.opt
.ti/generate/device.lds.genlibs
```

看到下面的输出表示生成成功：

```text
SysConfig Complete!
```

SysConfig 输出的 `info` 通常是芯片使用建议，不代表生成失败。应以脚本退出状态和是否生成上述文件为准。

### 3. 配置 CMake

Debug 构建：

```powershell
cmake --preset user-debug
```

Release 构建：

```powershell
cmake --preset user-release
```

配置成功后，构建文件位于 `build/`。

### 4. 编译工程

```powershell
cmake --build build
```

构建产物位于：

```text
build/outputs/<工程目录名>.elf
build/outputs/<工程目录名>.hex
build/outputs/<工程目录名>.map
```

其中：

- `ELF` 用于烧录和源码调试
- `HEX` 用于固件烧录
- `MAP` 用于分析符号和内存占用

裸机工程使用 `nosys.specs` 时，链接器可能提示 `_read`、`_write`、`_close` 或 `_lseek` 未实现。如果工程没有使用对应的标准输入输出功能，这些警告不影响固件生成。

## 使用 VS Code 一键构建

完成首次配置后，在 VS Code 中按：

```text
Ctrl + Shift + B
```

默认的 `CMake build` 任务会依次执行：

```text
TI Sysconfig Prebuild
    ↓
CMake configure
    ↓
CMake build
```

也可以通过“终端 → 运行任务”单独执行：

- `Project Configure`
- `TI Sysconfig Prebuild`
- `CMake configure`
- `CMake build`
- `CMake clean`
- `CMake clean rebuild`
- `Flash (DAPLink)`
- `Flash (J-Link)`
- `Flash (XDS110)`

## 使用 DAPLink 烧录

连接 DAPLink 与目标板，并确认 SWDIO、SWCLK、GND 和目标板供电连接正确。

在 VS Code 中运行：

```text
Flash (DAPLink)
```

或者在工程根目录手动执行：

```powershell
& "D:\Tools\OpenOCD\bin\openocd.exe" `
  -s "D:\Tools\OpenOCD\openocd\scripts" `
  -f ".vscode\openocd_daplink.cfg" `
  -c "program build/outputs/TI-MSPM0-Template.elf verify reset exit"
```

请将示例中的 OpenOCD 路径和 ELF 文件名替换为自己的实际值。

成功时应看到类似输出：

```text
CMSIS-DAP: Interface ready
Cortex-M0+ processor detected
Programming Finished
Verified OK
Resetting Target
```

## 使用 XDS110 烧录

XDS110 已经过 OpenOCD 实际烧录验证。连接目标板后，可以在 VS Code 中运行：

```text
Flash (XDS110)
```

对应的 OpenOCD 配置文件为：

```text
.vscode/openocd_xds110.cfg
```

## 使用 J-Link 烧录

模板包含 J-Link 的 OpenOCD 配置：

```text
.vscode/openocd_jlink.cfg
```

但 Windows 默认安装的 SEGGER J-Link 驱动通常不能被 OpenOCD 的
`libusb` 接口直接使用。使用 `Flash (J-Link)` 前，可能需要通过以下工具更换驱动：

- USBDriverTool
- Zadig

通常可将对应的 J-Link USB 接口驱动更换为：

- WinUSB
- libusbK

更换驱动后，重新连接 J-Link，再运行：

```text
Flash (J-Link)
```

> 注意：更换 J-Link 驱动后，SEGGER J-Link Commander、Ozone 或其他官方工具可能无法继续识别设备。需要使用 SEGGER 工具时，请在 USBDriverTool、Zadig 或 Windows 设备管理器中恢复原来的 SEGGER 驱动。更换驱动前应确认选择的是 J-Link 对应接口，避免误操作其他 USB 设备。

## 使用 VS Code 调试

1. 连接调试器和目标板。
2. 确认工程已经成功编译。
3. 打开 VS Code 的“运行和调试”面板。
4. 选择 `Debug (DAPLink)`。
5. 按 `F5`。

调试配置会启动 OpenOCD、连接 `arm-none-eabi-gdb`、下载 ELF，并运行到 `main`。

成功后可以：

- 在 C/C++ 源码中设置断点
- 单步执行
- 查看局部变量和全局变量
- 查看寄存器
- 查看调用栈
- 查看内存

使用 XDS110 时选择 `Debug (XDS110)`；J-Link 完成上述驱动调整后，选择
`Debug (J-Link)`。

## 更换芯片

重新运行：

```powershell
.\scripts\configure.bat
```

选择 `Y` 重新配置，并输入新的芯片型号。脚本会根据
`scripts/mspm0_chip_db.csv` 自动更新：

- 芯片宏定义
- 芯片家族
- Cortex-M 内核和 ARM 架构参数
- 浮点 ABI
- GCC 启动文件
- CMake 配置
- VS Code 配置

更换芯片后还应检查 `.ti/*.syscfg` 是否与新器件匹配，然后重新执行：

```powershell
.\.ti\syscfg.bat
cmake --preset user-debug
cmake --build build
```

## 修改外设配置

使用 TI SysConfig 打开：

```text
.ti\TI-MSPM0-Template.syscfg
```

修改引脚、时钟或外设后保存文件，再运行：

```powershell
.\.ti\syscfg.bat
cmake --build build
```

不要直接长期修改 `Core/Src/ti_msp_dl_config.c` 或
`Core/Inc/ti_msp_dl_config.h`，因为再次运行 SysConfig 时这些文件会被重新生成。

用户自己的业务代码建议放在 `User/` 目录。

## 常见问题

### `config.ini not found`

尚未完成首次配置。运行：

```powershell
.\scripts\configure.bat
```

### 找不到 `sysconfig_cli.bat`

填写的 SysConfig 路径不正确。确认填写的是 TI Sysconfig 根目录。

### 找不到 `.metadata\product.json`

TI MSPM0 SDK 路径不正确，或者 SDK 安装不完整。确认填写的是 MSPM0 SDK 根目录。

### 找不到 `arm-none-eabi-gcc.exe`

检查 GCC 安装目录中是否存在：

```text
bin\arm-none-eabi-gcc.exe
bin\arm-none-eabi-gdb.exe
```

然后重新运行配置脚本。

### CMake 找不到 DriverLib

确认 TI SDK 路径正确，并先运行：

```powershell
.\.ti\syscfg.bat
```

模板会读取 `.ti/generate/device.lds.genlibs` 并自动链接 SysConfig 请求的库。

### OpenOCD 找不到 DAPLink

检查：

- DAPLink 是否已连接电脑
- USB 驱动是否正常
- 调试器是否被其他程序占用
- SWDIO、SWCLK 和 GND 是否连接正确
- 目标板是否供电

### OpenOCD 找不到 J-Link

如果 J-Link 能被 SEGGER 官方工具识别，但 OpenOCD 无法识别，通常是 USB 驱动不兼容。

可使用 USBDriverTool 或 Zadig，将 J-Link 对应接口更换为 WinUSB 或 libusbK 驱动。操作时务必确认设备和接口选择正确。

如果更换后 SEGGER 官方工具无法识别 J-Link，需要恢复原来的 SEGGER 驱动。

### OpenOCD 提示未设置 adapter speed

模板未强制指定调试时钟时，OpenOCD 会回退到较低速率，例如 `100 kHz`。这通常仍可正常调试，只是烧录和下载速度较慢。

如需提高速度，可在对应的 OpenOCD 配置文件中加入合适的速率，例如：

```tcl
adapter speed 1000
```

具体速率取决于调试器、接线质量和目标板。

### VS Code 配置文件是否乱码

生成的 `.vscode/*.json` 应使用 UTF-8 编码。`settings.json` 是 VS Code 支持的 JSONC 格式，允许注释和尾逗号；某些严格 JSON 检查器可能会报告语法错误，但这不代表文件乱码。

## 已验证配置

本模板已经使用以下组合完成实际验证：

- MSPM0G3507
- TI MSPM0 SDK 2.10.00.04
- Arm GNU Toolchain 15.2
- CMake + Ninja
- OpenOCD
- DAPLink / CMSIS-DAP / SWD：烧录和 GDB 调试通过
- XDS110：OpenOCD 烧录通过
- J-Link：模板已提供配置，使用 OpenOCD 时需要更换兼容的 USB 驱动

验证内容包括：

- SysConfig 成功生成代码
- CMake 成功配置和编译
- 成功生成 ELF、HEX 和 MAP
- OpenOCD 成功识别 Cortex-M0+
- 固件烧录并校验通过
- GDB 成功连接目标并停在 `main`
