# AirsimX 兼容性矩阵

检查日期：2026-11-06。源码基线：`1ca93f6f77e4e8a39b2b241c1fe2764da4d7dd41`。

## 状态定义

- **源码基线**：配置、实现或历史文档中可确认，不表示本机已运行。
- **已检测**：本机存在该工具或组件，不表示项目兼容。
- **工具实测通过**：独立工具测试通过，不表示 AirSim 构建或仿真通过。
- **候选**：后续拟验证的组合，尚未承诺支持。
- **未验证**：没有本项目的构建或运行证据。

当前没有任何 UE5/现代飞控组合被标为正式支持。宿主机与 WSL 的记录见[开发环境记录](development-baseline.md)。

## 引擎与构建

| 组件 | 源码基线或本机情况 | 目标与验证状态 |
| --- | --- | --- |
| Unreal Engine | Blocks 绑定 UE 4.27 | UE5 精确版本待选；未试编译 |
| UE 物理/车辆 | Build.cs 依赖 PhysX、APEX、PhysXVehicles；uplugin 启用 PhysXVehicles | 分离车辆依赖，验证 Chaos 边界；未迁移 |
| 多旋翼动力学 | 存在 AirLib FastPhysicsEngine | SimpleFlight 核心测试已在两种配置通过；未接 Unreal 物理与碰撞 |
| Windows | Windows 11 x64，10.0.26200 | 首期开发平台；UE 未验证 |
| Visual Studio | Build Tools 2022 17.14.37，MSVC 工具集 14.44.35207 | Release/Debug 核心构建通过；UE 匹配未验证 |
| Windows SDK | 10.0.26100.0 | 已用于核心构建；与目标 UE 的匹配未验证 |
| CMake | VS 自带 3.31.6-msvc6 | rpclib 的 VS 生成器配置与构建通过；AirSim CMake 构建链未验证 |
| Ninja | VS 自带 1.12.1 | 版本命令通过；本次未使用 |
| MSBuild | 17.14.51.32402 | 原生 AirLib/MavLinkCom/测试项目两种配置构建通过；完整 AirSim.sln 未构建 |
| 原生依赖 | rpclib 2.3.0、Eigen 3.3.7 | 已在项目内准备并用于构建，归档摘要已记录 |
| AirLibUnitTests | 修复测试阻塞并补充断言 | Release/Debug 4 用例及超时/时区/参数路径 22 项验证通过 |
| Linux/其他平台 | 继承上游代码与工作流 | 不纳入 v0.1 正式支持承诺 |

## 飞控

| 组合 | 基线证据 | 候选目标 | 项目验证状态 |
| --- | --- | --- | --- |
| SimpleFlight | 内置实现 | UE5 首个飞行闭环 | 核心测试起飞/位置/重置通过；无 Unreal、碰撞或着陆验证 |
| PX4 SITL | 旧文档示例 v1.11.3 | v1.17.0 | 仅核实发布元数据；未构建、连接或飞行 |
| ArduPilot SITL | ArduCopterApi 使用 UDP 交换传感器/执行器数据，多个控制 API 未实现 | Copter-4.7.2 | 仅核实发布元数据；未构建、连接或飞行 |
| PX4 HITL | 继承旧说明 | 待 SITL 稳定后确定 | 未连接硬件 |
| 多机 | 存在上游代码与说明 | 单机通过后验证 | 未运行 |

PX4 v1.17.0 是查询当日 GitHub latest release 返回的非预发布版本。Copter-4.7.2 是查询到的最近 30 个 ArduPilot releases 中最新的非预发布 Copter 条目。它们是调研候选，不是已安装、已锁定或受支持的版本；实际采用时须重新核实标签、提交、固件文档与工具链。

## Python、ROS 与算法

| 组件 | 本机或源码基线 | 建议与状态 |
| --- | --- | --- |
| Windows Python | 默认 3.12.10；启动器还登记 3.11.15、3.8.20 | 首轮评估 Python 3.12 独立环境；旧 RPC 依赖兼容性未验证 |
| Python SDK | 包名 airsim；依赖 msgpack-rpc-python、numpy、opencv-contrib-python | 保留导入名；需固定依赖并测试 RPC/图像/异步行为 |
| WSL Ubuntu | 26.04，GCC 15.2.0，Python 3.14.4 | 不直接作为首个 PX4 正式基线 |
| 飞控 Linux 环境 | PX4 当前 main 文档列出 Ubuntu 24.04/22.04，明确 26.04 尚不支持 | 独立 Ubuntu 24.04 候选；本轮未安装 |
| ROS 2 | 仓库已有 ros2/src wrapper；Ubuntu 未发现 ros2/colcon 或 /opt/ros 安装 | Ubuntu 24.04 + Jazzy 候选；wrapper 未编译 |
| Gymnasium | 未做适配验证 | M3 规划，不宣称已支持 |
| 定位/规划/学习算法 | 继承部分示例 | 按算法独立环境、固定版本逐项验收 |

现有 ROS 2 README 指向的通用说明主要包含 ROS 1/catkin 步骤，不能直接作为现代 ROS 2 的安装验收依据。

## 正式支持记录要求

一条正式支持记录必须包含源码提交、引擎精确版本、编译工具链、操作系统、飞控标签及提交、Python 依赖锁定、场景/车辆/传感器配置、执行命令、通过条件和日志位置。构建通过、传感器通过与飞行通过应分别记录；网络中断、reset 和打包测试不得省略。

## 依据

- [Blocks 引擎配置](../../Unreal/Environments/Blocks/Blocks.uproject)
- [UE 模块依赖](../../Unreal/Plugins/AirSim/Source/AirSim.Build.cs)与[插件声明](../../Unreal/Plugins/AirSim/AirSim.uplugin)
- [Windows 构建脚本](../../build.cmd)与[Python 包声明](../../PythonClient/setup.py)
- [PX4 历史说明](../px4_sitl.md)
- [ArduCopter 实现](../../AirLib/include/vehicles/multirotor/firmwares/arducopter/ArduCopterApi.hpp)
- [ROS 2 包声明](../../ros2/src/airsim_ros_pkgs/package.xml)
- [PX4 v1.17.0](https://github.com/PX4/PX4-Autopilot/releases/tag/v1.17.0)
- [ArduPilot Copter-4.7.2](https://github.com/ArduPilot/ardupilot/releases/tag/Copter-4.7.2)
- [PX4 Ubuntu 开发环境](https://docs.px4.io/main/en/dev_setup/dev_env_linux_ubuntu.html)：滚动文档，采用特定固件时须复核版本对应说明。
- [ROS 2 Jazzy 平台支持](https://docs.ros.org/en/jazzy/Releases/Release-Jazzy-Jalisco.html)
- [UE5 迁移指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-migration-guide)
