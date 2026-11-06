# AirLib 核心构建与测试记录

日期：2026-11-06。源码基线：`1ca93f6f77e4e8a39b2b241c1fe2764da4d7dd41`。

**结论（更新）：Windows x64 Release、Debug 核心构建均成功；修复测试阻塞后，两种配置的 22 项验证全部通过，见下方“修复与复验”。** 首次执行时原始测试失败，记录保留在后文中，不做覆盖。

## 验证范围

使用现有 VS 2022 Build Tools 17.14.37、MSVC 19.44.35228（工具集目录 14.44.35207）、Windows SDK 10.0.26100.0、MSBuild 17.14.51.32402、VS 自带 CMake 3.31.6-msvc6。

采用原生 MSBuild 项目：构建 `AirLibUnitTests/AirLibUnitTests.vcxproj`，通过项目引用构建 AirLib 和 MavLinkCom。rpclib 使用 CMake 的 Visual Studio 17 2022/x64 生成器，保留默认动态 CRT，关闭 rpclib 自身测试及示例。本轮没有验证 AirSim 的 Linux CMake 构建链，也没有构建完整 AirSim.sln 的全部项目。

不安装 Unreal，不下载车辆资产，不运行根目录 build.cmd/clean_rebuild；PATH 只在构建子进程中临时设置，没有改动用户或系统配置。未提交、未推送。

## 项目内依赖

版本保持与继承的 build.cmd 一致，本轮不是依赖升级：

| 依赖 | 版本 | 项目内位置 |
| --- | --- | --- |
| Eigen | 3.3.7 | `AirLib/deps/eigen3`，保留完整解压包及许可文件 |
| rpclib 源码 | 2.3.0 | `external/rpclib/rpclib-2.3.0` |
| rpclib 头文件 | 2.3.0 | `AirLib/deps/rpclib/include` |
| rpclib 静态库 | x64 Release / Debug | `AirLib/deps/rpclib/lib/x64/<配置>/rpc.lib` |
| MavLinkCom | 仓库自带版本 | 原项目直接构建，不另下载 |

下载地址及本地计算的 SHA-256：

- rpclib：<https://github.com/rpclib/rpclib/archive/v2.3.0.zip>
  - `7ac226e6d66e919354c0ce757ea2a600158617eccfa1dc3f6f2f02329f0c66d2`
- Eigen：<https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.zip>
  - `4815118c085ff1d5a21f62218a3b2ac62555e9b8d7bacd5093892398e7a92c4b`

这些摘要是下载后的本地指纹，不是发布者签名或独立可信的供应链认证。归档分别存放在 `build/airsimx/downloads/` 下的独立目录；解压时检查路径越界和符号链接。下载包、解压目录与自写脚本分开，Python 使用 `-I` 运行。rpclib 配置生成后的 config.h/version.h 与复制头文件按文本比较一致，字节差异仅为换行格式。

## 构建结果

| 目标 | Release x64 | Debug x64 |
| --- | --- | --- |
| rpc.lib | 成功 | 成功 |
| MavLinkCom.lib | 成功 | 成功 |
| AirLib.lib | 成功 | 成功 |
| AirLibUnitTests.exe | 成功 | 成功 |

产物分别位于：

- `AirLib/lib/x64/<配置>/AirLib.lib`
- `MavLinkCom/lib/x64/<配置>/MavLinkCom.lib`
- `AirLib/deps/rpclib/lib/x64/<配置>/rpc.lib`
- `AirLibUnitTests/build/x64/<配置>/AirLibUnitTests.exe`

八个产物的大小与 SHA-256 已记录在本地 `build/airsimx/artifacts.json`。Release 核心 MSBuild 日志报告 3 个 C4100 未使用参数警告、0 错误；Debug 核心 MSBuild 日志报告 0 警告、0 错误。rpclib 的 Debug 构建另有 C4996/STL4043 弃用提示，CMake 也提示旧版本兼容策略；未屏蔽这些警告，不能把核心 MSBuild 的零警告推广到全部第三方构建。

## 原始测试结果

在独立项目内工作目录提供 settings.json，配置 SimpleFlight；运行前不修改用户已有的 AirSim 配置。每个原始测试进程设置 30 秒墙钟超时，并禁止系统崩溃弹窗阻塞自动执行。

| 原始程序 | 结果 | 退出码 | 实际耗时 |
| --- | --- | --- | --- |
| Release AirLibUnitTests.exe | 失败，断点异常 | `2147483651 / 0x80000003` | 约 0.015 秒 |
| Debug AirLibUnitTests.exe | 失败，断点异常 | `2147483651 / 0x80000003` | 约 0.047 秒 |

两次均在超时前失败，不是超时通过或正常结束。

为定位原因，在 `build/airsimx/scripts/individual-tests.cpp` 编写了独立诊断入口，调用原有测试类。它不替换上游 main.cpp，也不代表新增的正式测试套件。每个测试单独进程运行，15 秒超时：

| 原有测试类 | 单独执行结果 | 覆盖边界 |
| --- | --- | --- |
| QuaternionTest | 返回 0 | 当前 run() 只执行旋转计算，没有结果断言 |
| CelestialTest | `0x80000003` | 天体数值不满足断言，TestBase 调用 __debugbreak |
| SettingsTest | 返回 0 | 只加载 JSON，没有加载成功断言 |
| SimpleFlightTest | `0xc0000005` 访问异常 | 未完成车辆初始化；不能视为飞行验证 |

因此不能将两个返回 0 的案例表述为“数学和配置功能已全面通过测试”。原始完整套件被 CelestialTest 提前中断；SimpleFlight 的结果来自独立诊断入口。

## 已核实的失败原因与测试缺口

### 天体测试日期解析失败

[CelestialTests.hpp](../../AirLibUnitTests/CelestialTests.hpp) 使用 `2018-February-22 15:24:00`；[Utils.hpp](../../AirLib/include/common/common_utils/Utils.hpp) 的 to_time_t 默认格式是 `%Y-%m-%d %H:%M:%S`，tm 未零初始化，也未检查解析失败。

独立诊断确认该输入/格式组合解析失败。此次单测诊断中时间值转换为 uint64_t 后为 `18446744073709551615`，太阳高度约 9.84619、月球高度约 28.5419、月相比例约 0.780878，与测试期望不符。该无效时间值不应作为稳定结果依赖。另有 mktime 使用本地时区的问题，修复时还需明确时区语义，不能只改字符串后放宽断言。

### SimpleFlight 未初始化车辆配置

原始 SettingsTest 仅调用 Settings::loadJSonFile；原始 main.cpp 与 SimpleFlightTest 没有调用 AirSimSettings::load 来构建车辆表。[getVehicleSetting](../../AirLib/include/common/AirSimSettings.hpp) 在未找到车辆及后备 SimpleFlight 条目时直接解引用结束迭代器。

独立诊断确认：JSON 加载成功，但车辆数量为 0；显式调用 AirSimSettings::load 后才变为 1。结合原始测试路径与访问异常，定位到缺失车辆初始化这一阻塞点。本轮未据此修改正式实现或测试。

### SimpleFlight 本身不是有限时长测试

[SimpleFlightTest.hpp](../../AirLibUnitTests/SimpleFlightTest.hpp) 末尾存在 `while (true)`；即使修复前面的初始化，也不会正常退出。起飞与移动返回值同样未作为成功断言。它需要重构成有明确完成条件和超时的测试，而不是简单删除失败路径或把超时算成功。

## 修复与复验

在不安装 Unreal、不改变系统环境的前提下修复了上述问题。改动范围：日期解析工具、缺失车辆报错、测试运行器与四个测试、路径命令完成判定，以及 Unreal 调用点的异常兜底。

| 改动 | 说明 |
| --- | --- |
| `Utils::to_time_t` | tm 零初始化、固定 classic locale、解析失败抛 `std::invalid_argument`、拒绝多余尾随字符；保留本地时间/DST 语义 |
| `AirSimSettings::getVehicleSetting` | 找不到车辆时抛异常而非解引用 `end()` |
| `SimModeBase::setTimeOfDay` | 捕获解析异常，记录失败并回退当前时间，避免配置错误导致引擎崩溃；此文件属 Unreal 插件，本机未编译验证 |
| `TestBase::testAssert` | 断言失败抛异常，不再触发 `__debugbreak`，使失败可被运行器报告 |
| `main.cpp` | 每个用例独立墙钟看门狗（默认 60 秒，可配置 1–3600），超时以失败码退出；支持 `--test` 选择与参数校验；返回非零码 |
| CelestialTest | 使用固定 UTC 时间戳（Seattle PST 对应时刻），增加格式、往返与非法输入断言 |
| QuaternionTest | 增加旋转结果、归一化与逆变换断言 |
| SettingsTest | 覆盖非法 JSON、缺失车辆、车辆加载及 SimpleFlight 回退 |
| SimpleFlightTest | 消除 `while (true)`；显式加载设置与地面真值、`Kinematics/Environment` 重置、地面锁定，验证解锁、起飞、位置到达、有限状态、锁定超时失败、 disarm 与 reset；命令有仿真时限，外层有墙钟看门狗 |
| `moveOnPath` | 路径正常结束（非超时、未取消且到达终点容差内）时标记完成，修复本可成功却返回 `false` 的问题 |

修复过程中曾发现两个次生问题并一并解决：测试夹具构造后未调用 `reset()` 导致读取未初始化状态；路径循环因看门狗检查而退出时未置完成标志。

### 复验结果

最终代码在两种配置下重新构建后执行 `verify_fixed_tests.py`，每项均符合预期：

| 验证项 | Release | Debug |
| --- | --- | --- |
| 完整套件（4 用例） | 通过，约 6.0 秒 | 通过，约 6.4 秒 |
| Celestial 在 TZ=UTC0 / PST8 / CST-8 | 全部通过 | 全部通过 |
| SimpleFlight 连续运行两次 | 均通过，各约 5.8 秒 | 均通过，各约 5.8 秒 |
| 故意 `--timeout-seconds 1` | 返回 1 并输出 `[ TIMEOUT ]`，约 1.03 秒 | 相同 |
| 未知测试名 | 返回 2 | 返回 2 |
| 超时参数 0 / 1junk / 3601 | 均返回 2 | 均返回 2 |

结果文件：`build/airsimx/post-test-fix/results.json`；首次失败的证据备份在 `build/airsimx/pre-test-fix/`。

### 仍未覆盖

- SimpleFlight 夹具通过 `setGrounded(true)` 模拟地面，没有验证真实碰撞、着陆与 Unreal 物理。
- `SimModeBase.cpp` 的改动未在 Unreal 中编译或运行。
- 未验证 RPC 传输、Python 客户端、外部飞控、渲染、传感器标定或跨机器确定性。
- 修复不改变 AirSim 的本地时区语义：不同时区下配置的启动时间仍按各自本地时间解释，这是有意保留的行为。

## 本地复现与证据

本轮辅助脚本和原始日志都在被 Git 忽略的 `build/airsimx/` 中；它们用于当前机器诊断，不是已交付的跨机器构建系统。全新克隆不会自动包含这些本地文件。prepare_deps.py 会拒绝覆盖已存在的解压目标，不应在当前已准备状态下重复运行。

当前机器可重跑构建：

```bash
powershell.exe -NoProfile -Command '& "D:\Airsim\build\airsimx\scripts\core-build.cmd" Release; exit $LASTEXITCODE'
```

```bash
powershell.exe -NoProfile -Command '& "D:\Airsim\build\airsimx\scripts\core-build.cmd" Debug; exit $LASTEXITCODE'
```

当前机器可重跑完整验证矩阵：

```bash
python -I D:/Airsim/build/airsimx/scripts/verify_fixed_tests.py D:/Airsim
```

单独运行测试（修复后应返回 0）：

```bash
./AirLibUnitTests/build/x64/Release/AirLibUnitTests.exe --test SimpleFlight
```

证据位置：

- `build/airsimx/dependencies.json`：依赖来源及归档摘要。
- `build/airsimx/logs/build-Release.log`、`build-Debug.log`：完整构建输出。
- `build/airsimx/logs/core-Release.log`、`core-Debug.log`：MSBuild 详细日志。
- `build/airsimx/test-runs/Release/result.json`、`Debug/result.json`：原始测试退出码。
- `build/airsimx/test-runs/individual/results.json`：首次逐项诊断结果（备份于 `pre-test-fix/`）。
- `build/airsimx/logs/diagnose-inputs.log`：日期解析和车辆初始化诊断。
- `build/airsimx/post-test-fix/results.json`：修复后全部验证项及输出。

核心测试现已在两种配置下通过，但 M0 仍需依赖流程跨机器化与旧版参考回归。UE、飞控、Python RPC、ROS 2、碰撞与传感器闭环均未验证，不能据此宣称完整仿真可用。
