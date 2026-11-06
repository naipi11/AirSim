# 本地开发环境记录

检查日期：2026-11-06。本文保留首次克隆后的环境快照，不是当前构建状态清单。

> 后续更新：同日已准备项目内 Eigen/rpclib，完成 Release、Debug 核心构建；原始测试失败后已修复并在两种配置下通过。本文下方“依赖缺失”“尚未构建”等描述是首次检查时的历史状态，最新结果以[核心构建与测试记录](core-validation.md)和[兼容性矩阵](compatibility.md)为准。

## 仓库

- 位置：`D:/Airsim`。
- 完整克隆，无 shallow 限制；保留远程可达历史。
- `origin`：`https://github.com/naipi11/AirSim.git`。
- `upstream`：`https://github.com/microsoft/AirSim.git`，仅新增本地 remote 配置，未推送或改动上游。
- 起点：`1ca93f6f77e4e8a39b2b241c1fe2764da4d7dd41`，提交日期 2026-09-15。
- 克隆时 fork 的 main 与查询到的微软 main 提交一致。
- 本地工作分支：`docs/airsimx-baseline`；本地 main 保持克隆时的起点。
- 未发现仓库内 CLAUDE.md 或 AGENTS.md；未发现 .gitmodules，submodule status 无条目。
- 本轮仅新增开发文档及 README 入口，不修改构建脚本、引擎工程、源码、资产、许可证或远程仓库名称；不提交、不推送。

## Windows 环境

| 项目 | 检测结果 |
| --- | --- |
| 系统 | Windows 11 Pro x64，10.0.26200 |
| CPU | Intel Core Ultra 7 255HX，20 核/20 逻辑处理器 |
| 内存 | 约 31.4 GiB 系统可见内存 |
| 独立 GPU | NVIDIA GeForce RTX 5060 Laptop GPU；驱动 32.0.16.1062 |
| D 盘剩余空间 | 检查时约 1.32 TiB，动态变化，不是容量保证 |
| Git | 2.55.0.windows.2 |
| GitHub CLI | 2.96.0 |
| Visual Studio | Build Tools 2022，17.14.37；未检测到完整 VS IDE 实例 |
| MSVC | 工具集目录 14.44.35207；编译器 19.44.35228 |
| Windows SDK | 10.0.26100.0 |
| CMake / Ninja | VS 自带 3.31.6-msvc6 / 1.12.1 |
| MSBuild | 17.14.51.32402 |
| .NET SDK | 10.0.401；另检测到多个 7/8/10 runtime，不据此判断 UE 可用 |
| 默认 Python | 3.12.10 |
| 其他 Python | py 启动器登记 3.11.15、3.8.20；未逐一启动验证 |
| Unreal Engine | PATH、Epic 注册表、Launcher 安装清单及 UE 环境变量中未发现 |

未做全盘搜索，因此“未发现 Unreal”不等于证明机器上不存在手动放置的源码引擎。具体 UE 路径和版本仍需确定。硬件信息只证明设备可见，尚未验证 Unreal 渲染性能、显存预算或驱动兼容性。

### 工具路径与实测

VS 安装目录为 `C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools`。MSVC、MSBuild、CMake、Ninja 未全部加入普通终端 PATH，但安装文件存在；不应因此重新安装整套工具链。

本轮在独立临时目录运行最小 C++17 程序，使用 VS 的 `VC/Auxiliary/Build/vcvars64.bat` 初始化子进程环境，执行编译、链接和程序运行。结果：退出码 0，输出 `AirsimX compiler smoke test: 6`。

过程中发现并处理了两项调用问题：

- 只运行 `cl /Bv` 会因没有源文件返回 D8003，这不代表编译器损坏。
- 使用子进程临时 PATH 加入 VS Installer 目录后，`vswhere` 提示消失；程序使用显式相对路径执行。

没有修改用户或系统 PATH。临时测试不是 AirLib、Unreal 或飞控测试，也没有安装项目依赖。

### Python 依赖快照

只检查默认 Python 3.12 环境的包元数据，不导入或安装项目包：检测到 numpy 2.5.1；未检测到 airsim、msgpack-rpc-python、opencv-contrib-python、mkdocs 或 pymdown-extensions。这不代表其他虚拟环境也没有这些包。

建议后续使用项目独立环境并锁定依赖，不直接改变全局 Python。旧 RPC 栈与现代 Python/NumPy 的兼容性尚未验证。

## WSL 与飞控环境

- WSL 2.7.10.0，内核 6.18.33.2-2。
- 登记有 deploy、putn、Ubuntu 三个 WSL2 发行版；检查开始时均停止。
- 仅启动 Ubuntu 做只读工具检查，未检查或修改另外两个发行版，也未关闭整个 WSL。
- Ubuntu 为 26.04 LTS；GCC 15.2.0、Python 3.14.4、Git 2.53.0；g++、make 可从 PATH 找到。
- Ubuntu PATH 未找到 cmake、ninja、clang、px4、sim_vehicle.py、ros2、colcon；/opt/ros 未发现发行版目录。
- Ubuntu 中存在 docker 命令，但没有验证 daemon、镜像或容器可用性。

PATH 检查不是全盘源码搜索：不能据此断言所有位置都没有 PX4/ArduPilot。没有执行固件构建、下载、仿真连接或硬件操作。

PX4 当前官方开发文档列出 Ubuntu 24.04/22.04，明确 26.04 尚不支持。因此建议另行准备 Ubuntu 24.04，不降级或重装现有 Ubuntu。本轮没有安装该候选环境。

## 已验证与未验证

已验证：Git 远程访问和完整克隆、基线提交、本地分支及 remote 配置、工具版本、独立 MSVC 编译/链接/运行、Ubuntu 系统和工具可见性。

未执行：AirLib 构建与单元测试、Python SDK 安装/RPC 测试、Unreal 编译与打包、飞行/传感器测试、PX4/ArduPilot 构建与连接、ROS 2 构建、MkDocs 站点构建。

未运行旧版 build.cmd/setup.sh/clean_rebuild 脚本。build.cmd 会下载 rpclib、Eigen 和可选车辆资源，调用镜像复制并更新场景插件；check_cmake.bat 在找不到 CMake 时可能启动安装流程。当前应先做依赖准备方案，不将这些脚本当作无副作用的环境检查命令。

## 环境缺口与后续顺序

1. 先明确 Unreal 安装路径和精确候选版本，再匹配 MSVC/SDK；本机已有组件不等于已满足该引擎的要求。
2. 固定并准备 Eigen、rpclib 等依赖，验证 AirLib 构建与测试；克隆后的相关 deps/external 目录尚不存在。
3. 建立 Python 独立环境，验证旧客户端兼容性。
4. 为现代飞控另行准备受支持的 Linux 环境；ROS 2 Jazzy 与 Ubuntu 24.04 作为候选组合。
5. 按[路线图](roadmap.md)推进，每次构建或飞行验证后更新[矩阵](compatibility.md)。

以上是下一阶段准备事项，并非本轮已执行的安装或迁移。
