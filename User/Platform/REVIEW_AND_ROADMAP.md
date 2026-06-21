# Platform 层审查报告与改进路线图

> 第一次审查：2026-06-20
> 第二次审查：2026-06-20（第二版）
> 审查范围：`User/Platform/` 全部源码
> 审查标准：嵌入式 C 编码规范 + 工业级产品要求（IEC 61508 SIL-2 参考）

---

## 一、总览

Platform 层是 MCU 对外的接口契约层，参考 Linux 驱动模型设计：

```
Application
    |
Board        静态设备枚举与资源分配
    |
Device       电机、编码器、TB6612FNG、传感器
    |
Platform     GPIO、定时器、通信子系统契约
    |
arch/MSPM0   TI DriverLib 提供者
```

| 审查轮次 | 综合评分 | 变化 |
|:---:|:---:|:---:|
| 第一版 | **7.2 / 10** | 准工业级，架构和基本功扎实 |
| 第二版 | **7.9 / 10** | 类型统一 + 目录重组 + 测试框架扩展 |

补齐测试覆盖、可观测性、静态分析后再做一轮验证，可以达到 IEC 61508 SIL-2 量产水平。

---

## 二、当前得分详情（第二版）

| 维度 | 第一版 | 第二版 | 变化 | 状态 | 说明 |
|------|:---:|:---:|:---:|:---:|------|
| 架构抽象 | 9.5 | 9.5 | — | ✅ | Linux subsystem API 风格，ops 表 + 注册模式，分层严格 |
| 防御性编程 | 8.5 | 8.5 | — | ✅ | NULL 保护、范围校验、成对性校验（attach/detach 全有或全无） |
| 错误处理 | 8.0 | 8.0 | — | ✅ | 9 种统一状态码，不吞错误，CAN 帧校验在 dispatcher 层完成 |
| ISR 安全 | 9.0 | 9.0 | — | ✅ | 事件循环上限、临界区短促、IRQ 上下文保护正确 |
| 代码风格 | 9.0 | **9.5** | +0.5 | ✅ | `int` → `platform_status_t`；`system/` → `core/`+`gpio/` 语义更清晰 |
| 文档 | 8.0 | **8.5** | +0.5 | ✅ | 新增 `analog/README.md`，README 更新为最新目录结构 |
| 测试覆盖 | 4.0 | **6.0** | +2.0 | ⚠️ | 全部 10 个 dispatcher 编译验证；所有子系统有缺失 provider 测试 |
| 可观察性 | 2.0 | 2.0 | — | 🔴 | 几乎无可视化健康监控和运行时诊断能力 |
| 静态分析准备 | 5.0 | 5.0 | — | ⚠️ | `-Wall -Wextra -Werror -pedantic` 零警告，但缺 MISRA / Clang-Tidy |
| 可移植性 | 8.5 | 8.5 | — | ✅ | arch 层完美隔离，换 MCU 只需写新 `arch/X/` 目录 |

**综合：7.2 → 7.9 / 10（+0.7）**

---

## 三、已做对的事情（保持）

### 3.1 Linux 风格的分层设计

- 依赖方向严格向下，Device 层不接触 TI 头文件
- 不透明 Platform ID：Board 分配，MSPM0 provider 是唯一知道 ID→外设映射的代码
- 刻意省略了不需要的 Linux 特性：热插拔、sysfs、引用计数、动态分配、驱动匹配

### 3.2 按服务分离的公共契约

一个物理定时器可支持多个服务（PWM / Clock / Capture），消费者只看服务契约：

| 子系统 | 文件 | Ops 表成员数 | 可选能力 |
|--------|------|:---:|------|
| GPIO | `gpio/platform_gpio.*` | 7 | IRQ 四个函数全有或全无 |
| IRQ 临界区 | `core/platform_irq.h` | 2 | - |
| 状态 | `core/platform_status.h` | 10 | - |
| Clock | `timers/platform_clock.*` | 2 | - |
| PWM | `timers/platform_pwm.*` | 3 | - |
| Counter | `timers/platform_counter.*` | 6 | get_direction 可选 |
| Input Capture | `timers/platform_input_capture.*` | 7 | attach/detach 可选 |
| Periodic Timer | `timers/platform_periodic_timer.*` | 6 | - |
| UART | `communications/platform_uart.*` | 5 | attach/detach 可选 |
| I2C | `communications/platform_i2c.*` | 1 | - |
| CAN | `communications/platform_can.*` | 5 | get_status 可选 |
| SPI | `communications/platform_spi.*` | 1 | - |
| ADC | `analog/` | 保留 | 契约尚未定义 |
| DAC | `analog/` | 保留 | 契约尚未定义 |

### 3.3 统一状态码

```c
typedef enum platform_status {
    PLATFORM_STATUS_OK                    = 0,
    PLATFORM_STATUS_INVALID_ARGUMENT      = -1,
    PLATFORM_STATUS_NO_DEVICE             = -2,
    PLATFORM_STATUS_NOT_SUPPORTED         = -3,
    PLATFORM_STATUS_BUSY                  = -4,
    PLATFORM_STATUS_TIMEOUT               = -5,
    PLATFORM_STATUS_IO_ERROR              = -6,
    PLATFORM_STATUS_TRY_AGAIN             = -7,
    PLATFORM_STATUS_INVALID_STATE         = -8,
    PLATFORM_STATUS_ALREADY_REGISTERED    = -9,
} platform_status_t;
```

对标 Linux errno（`EINVAL / ENODEV / EOPNOTSUPP / EBUSY / ETIMEDOUT / EIO / EAGAIN`），但语义更明确。

### 3.4 ISR 编写规范

- 所有 ISR 有事件循环上限（32 / 8），防硬件故障饿死主循环
- `platform_irq_save/restore` 保护共享数据，临界区极短
- I2C / CAN 使用 `acquire/release` 机制防止重入
- UART ISR 双层循环：外层 32 事件 + 内层 16 RX 字节

### 3.5 正确的防御性设计

- `platform_gpio_ops_are_valid()`：IRQ 回调成对性校验
- `platform_input_capture_register()`：`(ops->attach == NULL) != (ops->detach == NULL)` 判定失败
- `platform_uart_register()`：同上
- `platform_can_frame_is_valid()`：完整帧校验（扩展 ID 范围、FD 不允许远程帧、BRS/ESI 只在 FD 帧）
- `platform_pwm_apply()`：dispatcher 层校验 `period_ticks == 0` 和 `pulse_ticks > period_ticks`
- I2C / CAN 所有硬件等待都有超时保护

### 3.6 MSPM0 硬件提供者亮点

- GPIO ISR dispatch 有 pin_index 范围保护
- PWM apply 正确处理同一 timer 的多 channel period 共享约束（返回 BUSY）
- UART 波特率计算使用 64× 分数分频 + 四舍五入
- I2C 错误后执行 `mspm0_i2c_recover()`（reset + flush TX/RX FIFO）
- CAN DLC 编解码使用查找表，正确覆盖 CAN FD 非线性长度
- CAN send 检查控制器是否在 NORMAL 模式

---

## 四、发现的问题

### 4.1 返回类型不一致 ✅ 已修复（第一版 → 第二版）

**原现象：** dispatcher `.c` 文件中函数返回类型是 `int`，但声明和语义是 `platform_status_t`。

**当前状态：** dispatcher、ops、provider 初始化入口和 fake provider
均已使用 `platform_status_t`；`platform_status_t` 自身已改为独立 `enum` 类型（零弱类型风险）。

### 4.2 目录结构重组 ✅ 已完成（第一版 → 第二版）

**原结构：**
```
Platform/
├── system/
│   ├── platform_status.h
│   ├── platform_irq.h
│   └── platform_gpio.*      ← GPIO 和 IRQ/状态混在 system/
```

**新结构：**
```
Platform/
├── core/
│   ├── platform_status.h    ← 纯粹的跨子系统类型
│   └── platform_irq.h
├── gpio/
│   └── platform_gpio.*      ← GPIO 作为独立子系统
├── analog/
│   └── README.md            ← 预留 ADC/DAC 契约
```

所有 include 路径已同步更新（arch 层使用 `../../core/`, `../../gpio/`；子系统的互引用使用 `../core/`）。

### 4.3 测试编译覆盖扩展 ✅ 已完成（第一版 → 第二版）

**第一版 CMakeLists** 只编译了 GPIO + SPI + Input Capture 3 个 dispatcher。

**第二版 CMakeLists** 编译全部 10 个 dispatcher：

```
gpio/platform_gpio.c
timers/platform_clock.c
timers/platform_counter.c
timers/platform_pwm.c
timers/platform_input_capture.c
timers/platform_periodic_timer.c
communications/platform_spi.c
communications/platform_uart.c
communications/platform_i2c.c
communications/platform_can.c
```

**13 源文件零警告编译通过**（`-Wall -Wextra -Werror -pedantic`），`test_missing_providers()` 已覆盖全部 10 个子系统的 `NOT_SUPPORTED` 路径。

### 4.4 `mspm0_gpio_irq_enable` 硬编码端口 A 的 NVIC 使能 🟡（第二版新发现）

**文件：** `arch/MSPM0/mspm0_gpio.c:194`

```c
static platform_status_t mspm0_gpio_irq_enable(platform_gpio_pin_t pin)
{
    // ...
    DL_GPIO_enableInterrupt(port, GPIO_PIN_MASK(pin));
    NVIC_EnableIRQ(GPIOA_INT_IRQn);   // ⚠️ 只使能了 GPIOA！
    return PLATFORM_STATUS_OK;
}
```

**分析：** 当 Port B 的 pin 调用 `irq_enable` 时，代码只使能了 GPIOA 的 NVIC 中断线。目前因为 Port A 和 Port B 共享 `GROUP1_IRQHandler`，软件行为正确——但如果未来需要独立的 GPIOA / GPIOB 中断优先级，这里会出问题。

**修复建议：**
```c
NVIC_EnableIRQ(port_index == 0U ? GPIOA_INT_IRQn : GPIOB_INT_IRQn);
```

**严重程度：** 低（当前硬件配置下不影响功能，但属于正确性隐患）。

### 4.5 `mspm0_gpio_dispatch` 中 callback 读取的 TOCTOU 风险 🟡（第二版新发现）

**文件：** `arch/MSPM0/mspm0_gpio.c:258-265`

```c
entry = &s_gpio_irq[port_index][pin_index];
callback = entry->callback;    // 一次读取
context = entry->context;      // 可能不是同一时刻的值
DL_GPIO_clearInterruptStatus(port, 1UL << pin_index);
if (callback) {
    callback(pin_id, context); // callback 和 context 可能不配对
}
```

**分析：** 在 ISR 上下文中不会被主线程抢占，但如果相同 GROUP1 ISR 在另一 Port 上嵌套触发，或硬件在清除中断状态后立即又触发了新中断，`callback` 和 `context` 可能来自不同的 `irq_attach` 写入。虽然概率极低，但工业级代码应该使用局部变量一次性快照 `volatile` 字段。

**严重程度：** 低（当前硬件单核 + 同一 GROUP ISR 不嵌套，实际不会触发）。但建议在回调读取处加注释说明"ISR 上下文：不抢占，一次读取即安全"以消除疑问。

### 4.6 测试覆盖仍不足（高优先级 — 质量保障）

**2026-06-20 第二版更新：** 主机测试已编译全部 dispatcher 并覆盖缺失 provider 行为。但状态化的正常 dispatch 测试仍只有 fake GPIO provider，更大范围的 fake provider 缺失仍待解决。

**当前状态：**

| 子系统 | 编译验证 | 缺失 Provider 测试 | Fake Provider | 正向 Dispatch 测试 |
|--------|:---:|:---:|:---:|:---:|
| GPIO | ✅ | ✅ | ✅ `platform_fake_gpio` | ✅ |
| Clock | ✅ | ✅ | ❌ | ❌ |
| PWM | ✅ | ✅ | ❌ | ❌ |
| Counter | ✅ | ✅ | ❌ | ❌ |
| Input Capture | ✅ | ✅ | ❌ | ❌ |
| Periodic Timer | ✅ | ✅ | ❌ | ❌ |
| SPI | ✅ | ✅ | ❌ | ❌ |
| UART | ✅ | ✅ | ❌ | ❌ |
| I2C | ✅ | ✅ | ❌ | ❌ |
| CAN | ✅ | ✅ | ❌ | ❌ |
| IRQ | N/A (依赖 CMSIS) | N/A | N/A | N/A |

**修复方向：** 每个子系统至少覆盖：
1. ✅ 缺失 provider 返回 `NOT_SUPPORTED` **（第二版已完成）**
2. NULL 参数返回 `INVALID_ARGUMENT`
3. 无效 ID 返回 `NO_DEVICE`
4. 超出范围的枚举值返回 `INVALID_ARGUMENT`
5. 正常注册 → 正常 dispatch → 结果验证
6. 重复注册返回 `ALREADY_REGISTERED`
7. 可选能力（attach/detach/get_direction/get_status）缺失时的正确行为

### 4.7 缺少运行时可观测性（高优先级 — 工业级必备）

**当前状态：** 平台层是被动响应式的。应用层调用 → 返回 OK 或 error code。但如果硬件悄然故障（晶振停振、总线锁死、CAN bus-off），没有主动通知机制。

**需要增加：**

```c
/* 建议新增：每个子系统的诊断结构体 */

/* GPIO 诊断 */
struct platform_gpio_diagnostics {
    uint32_t irq_storm_count;        /* 中断风暴检测 */
    uint32_t unexpected_level_count; /* 输出电平与预期不匹配 */
    bool     port_clock_ok;          /* 端口时钟状态 */
};

/* CAN 诊断 */
struct platform_can_diagnostics {
    uint32_t tx_timeout_count;       /* 发送超时次数 */
    uint32_t rx_overflow_count;      /* RX FIFO 溢出累计 */
    uint32_t bus_off_count;          /* bus-off 事件累计 */
    uint32_t last_error_timestamp;   /* 最后一次错误时间戳 */
};

/* 通用健康检查回调 */
typedef platform_status_t (*platform_diagnostics_callback_t)(
    void *diagnostics,
    void *context);
```

**目标：** 每个子系统至少有一个 `get_diagnostics()` 入口。

### 4.8 缺少故障注入能力（中优先级 — 可测试性）

**当前状态：** 测试时 fake provider 只能整体替换，不能在 dispatcher 层注入特定错误来验证 dispatcher 自己的错误处理逻辑。

**建议方案：**
```c
/* 测试专用：注入故障到 dispatcher 层 */
#if defined(PLATFORM_TEST_FAULT_INJECTION)
platform_status_t platform_gpio_fault_inject(
    platform_gpio_pin_t pin,
    platform_status_t forced_status);
#endif
```

### 4.9 缺少 MISRA / 静态分析验证（中优先级 — 合规）

**当前状态：** 通过了 `-Wall -Wextra -Werror -pedantic`，但没有运行过 MISRA-C 2012 检查。

**修复方向：**
1. 集成 `cppcheck`（免费）到 CMake 构建
2. 集成 `clang-tidy` 用于额外的静态分析检查
3. 如果有预算，跑一轮 PC-Lint 或 Coverity
4. 目标：零 MISRA-C 2012 Mandatory 违规

### 4.10 缺少栈分析（中优先级 — 实时安全）

**当前状态：** 没有对函数调用链的最大栈深度进行分析。

**修复方向：**
1. 启用 `-fstack-usage` 编译选项
2. 生成每个函数的栈使用报告
3. 分析 ISR 嵌套情况下的最大栈深度
4. 在链接脚本中预留足够的栈空间（建议 +20% margin）

---

## 五、改进路线图

### 2026-06-21 implementation update

- GPIO IRQ enable now selects IRQn from a static per-port descriptor containing
  the GPIO registers, IRQn and Platform port index. GPIOA/GPIOB remain separate
  descriptors even though MSPM0G3507 currently assigns both IRQn values to the
  same interrupt line.
- GPIO, UART, I2C and CAN now provide optional static diagnostics counters with
  atomic snapshot and reset operations.
- Diagnostics record history only. They do not reset hardware or call user
  callbacks. A normal API status describes one operation; diagnostics support
  longer-term logging, warning thresholds, degraded operation and explicit
  recovery/reset decisions made by Application.
- Host fake providers cover GPIOA/GPIOB dispatch, ISR-limit accounting and the
  diagnostics query/reset contracts. Reusable Platform examples live under
  `examples/` and are excluded from the default firmware image.

### Phase 1：类型整洁 + 目录重组 ✅ 已完成

```
✅ 将所有 dispatcher .c 文件的返回类型从 int 改为 platform_status_t
✅ platform_status_t 改为独立 enum 类型
✅ system/ 拆分为 core/ + gpio/
✅ 新增 analog/ 预留 ADC/DAC 契约
✅ 所有 include 路径同步更新
□ 集成 cppcheck 到 CMake 构建
□ 集成 clang-tidy（如果工具链支持）
□ 确认多编译器零警告（ARM GCC + MSVC 已验证通过）
```

### Phase 2：测试框架扩展 ✅ 部分完成

```
✅ 全部 10 个 dispatcher 加入 CMake 编译
✅ 所有子系统缺失 provider 测试
✅ 13 源文件零警告编译通过（-Wall -Wextra -Werror -pedantic）
□ 为 PWM、Clock、Counter、Periodic Timer 编写 fake provider 和合约测试
□ 为 UART、I2C、CAN 编写 fake provider 和合约测试
□ 添加可选能力缺失测试
□ 添加边界条件测试（length=0、baud=0、period=0 等）
□ 添加重复注册测试
□ 设置代码覆盖率门槛（目标 ≥ 90% 行覆盖）
```

### Phase 3：代码正确性修复（1 天）

```
□ 修复 mspm0_gpio_irq_enable 中硬编码 GPIOA NVIC 的问题（见 4.4）
□ mspm0_gpio_dispatch callback 读取添加注释或局部 volatile 快照（见 4.5）
```

### Phase 4：可观测性系统（2-3 天）

```
□ 设计通用的 platform_diagnostics 接口
□ 为每个子系统增加 get_diagnostics() 入口
□ MSPM0 实现：统计 ISR 风暴、总线错误、超时事件
□ 提供周期性健康检查回调（可选项）
```

### Phase 5：故障注入框架（1-2 天）

```
□ 设计 PLATFORM_TEST_FAULT_INJECTION 宏机制
□ 为 dispatcher 层增加故障注入点
□ 编写故障注入测试用例
□ 验证上层代码对所有错误码的响应正确性
```

### Phase 6：合规与交付（2-3 天）

```
□ 运行 MISRA-C 2012 检查并修复 Mandatory 违规
□ 生成栈使用报告，确认安全余量
□ 编写 Platform 层用户手册（初始化顺序、线程模型、回调上下文、清理要求）
□ 硬件回环测试：UART、I2C（有从机时）、CAN（有收发器时）
```

---

## 六、硬性规则对照检查

对照嵌入式 C 编码规范的硬性规则，Platform 层当前状态：

| 规则 | 限制 | 当前状态 |
|------|------|:---:|
| 最大函数长度 | 80 行 | ✅ 全部通过 |
| 最大嵌套深度 | 4 层 | ✅ 全部通过 |
| 最大行宽 | 80 列 | ⚠️ 部分行超过，需逐文件检查 |
| 最大函数参数数 | 5 个 | ✅ 全部通过（PWM state 等用结构体封装） |
| 魔法数字 | 禁止 | ✅ 全部使用宏 |
| 死代码 | 必须删除 | ✅ 无死代码 |
| 非公共符号 | 必须 `static` | ✅ 全部内部函数和变量为 static |
| const 正确性 | 未修改的指针用 const | ✅ ops 表、状态参数等正确使用 const |
| 头文件保护宏 | 必须有 | ✅ 全部有 `#ifndef` / `#define` / `#endif` |
| API 注释语言 | 英文/中文 | ✅ 英文注释 |
| 返回类型正确性 | `platform_status_t` 而非 `int` | ✅ 第二版已全量修复 |

---

## 七、目录结构清单

```text
Platform/
├── REVIEW_AND_ROADMAP.md          ← 本文件
├── README.md                      ← 设计文档
├── platform_init.c/h              ← 入口，委托到 arch
│
├── core/                          ← 跨子系统基础契约
│   ├── platform_status.h          ✅ 统一状态码（独立 enum）
│   └── platform_irq.h             ✅ IRQ 临界区
│
├── gpio/                          ← GPIO 子系统
│   └── platform_gpio.c/h          ✅ 数字 I/O + 边沿中断
│
├── timers/                        ← 定时器子系统
│   ├── platform_clock.c/h         ✅ 自由运行时钟
│   ├── platform_pwm.c/h           ✅ PWM 输出
│   ├── platform_counter.c/h       ✅ 计数器 / QEI
│   ├── platform_input_capture.c/h ✅ 输入捕获
│   └── platform_periodic_timer.c/h✅ 周期定时器
│
├── communications/                ← 通信子系统
│   ├── platform_uart.c/h          ✅ 异步串口
│   ├── platform_i2c.c/h           ✅ I2C 主控制器
│   ├── platform_can.c/h           ✅ CAN / CAN FD
│   └── platform_spi.c/h           ✅ SPI（契约已定义，缺 provider）
│
├── analog/                        ← 模拟子系统（预留）
│   └── README.md                  🔵 ADC/DAC 契约待定义
│
├── arch/                          ← MCU 架构提供者
│   ├── platform_arch.h            ✅ 架构抽象接口
│   └── MSPM0/
│       ├── mspm0_internal.h       ✅ ARRAY_COUNT 宏
│       ├── mspm0_providers.h      ✅ 提供者注册声明
│       ├── mspm0_resource_ids.h   ✅ GPIO ID 编码
│       ├── mspm0_platform.c       ✅ 提供者初始化遍历
│       ├── mspm0_gpio.c           ✅ GPIO + ISR（1 个待修复问题）
│       ├── mspm0_timer.c          ✅ PWM/Clock/Periodic 合并
│       ├── mspm0_uart.c           ✅ UART + ISR
│       ├── mspm0_i2c.c            ✅ I2C（同步阻塞，有超时）
│       ├── mspm0_can.c            ✅ CAN（有模式/状态管理）
│       └── mspm0_irq.c            ✅ IRQ save/restore
│
└── tests/                         ← 主机侧测试
    ├── README.md
    ├── CMakeLists.txt              ← 编译全部 10 个 dispatcher
    ├── fake/
    │   └── platform_fake_gpio.c/h ✅ Fake GPIO 提供者
    └── unit/
        └── platform_contract_test.c✅ 全部子系统缺失 provider 测试
```

---

## 八、行动计划（更新版）

按优先级排列：

| 优先级 | 任务 | 预计工时 | 状态 | 对工业级的贡献 |
|:---:|------|:---:|:---:|---|
| 🔴 P0 | 补齐测试覆盖（fake provider × 7 + 正向测试） | 3-5 天 | ⬜ | 质量保障根基 |
| 🔴 P0 | 增加运行时诊断接口 | 2-3 天 | ⬜ | 可观测性根基 |
| 🟡 P1 | 修复 mspm0_gpio NVIC 硬编码 | 0.5 天 | ⬜ | 正确性 |
| 🟡 P1 | mspm0_gpio_dispatch TOCTOU 注释/修复 | 0.5 天 | ⬜ | 代码可审查性 |
| 🟡 P1 | 集成静态分析（cppcheck + clang-tidy） | 1 天 | ⬜ | 合规准备 |
| 🟡 P1 | 栈分析 + 文档 | 1 天 | ⬜ | 安全基础 |
| 🟢 P2 | 故障注入框架 | 1-2 天 | ⬜ | 可测试性提升 |
| 🟢 P2 | MISRA-C 检查 | 1-2 天 | ⬜ | 合规验证 |
| 🟢 P2 | 硬件回环测试 | 1 天 | ⬜ | 实际验证 |

### 已完成的里程碑

| 任务 | 状态 |
|------|:---:|
| `int` → `platform_status_t` 全量替换 | ✅ |
| `platform_status_t` 改为独立 `enum` 类型 | ✅ |
| `system/` 拆分为 `core/` + `gpio/` | ✅ |
| 新增 `analog/` 预留 ADC/DAC | ✅ |
| 全部 include 路径同步更新 | ✅ |
| CMakeLists 编译全部 10 个 dispatcher | ✅ |
| 全部子系统缺失 provider 测试 | ✅ |
| 13 源文件 -Wall -Wextra -Werror -pedantic 零警告 | ✅ |

**总预计剩余工时：约 10-15 个工作日**，可将 Platform 层从准工业级提升到可以在 Class B / SIL-2 产品上使用的水平。

---

## 九、参考资料

- Linux Platform Devices and Drivers: <https://docs.kernel.org/driver-api/driver-model/platform.html>
- Linux GPIO consumer interface: <https://docs.kernel.org/driver-api/gpio/consumer.html>
- Linux PWM interface: <https://docs.kernel.org/driver-api/pwm.html>
- Linux Generic Counter interface: <https://docs.kernel.org/driver-api/generic-counter.html>
- Linux I2C subsystem: <https://docs.kernel.org/driver-api/i2c.html>
- Linux SocketCAN: <https://docs.kernel.org/networking/can.html>
- IEC 61508-3:2010 — Functional safety of E/E/PE safety-related systems (software)
- MISRA-C:2012 — Guidelines for the use of the C language in critical systems

---

*报告生成：Claude Code · 嵌入式 C 编码规范 Skill · 第一版 2026-06-20 · 第二版 2026-06-20*
