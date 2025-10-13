# SolveSpaceLib 集成说明

## 概述

本项目已成功集成 SolveSpaceLib 几何约束求解器库。SolveSpaceLib 提供了强大的2D和3D几何约束求解能力。

## 项目结构

```
Mathor/
├── main.cpp                    # 主程序入口，包含测试代码
├── main.qml                    # QML用户界面
├── Mathor.pro                  # Qt项目文件（已配置SolveSpaceLib）
├── GeometrySolver.h            # C++封装类头文件
├── GeometrySolver.cpp          # C++封装类实现
└── SolveSpaceLib/              # SolveSpaceLib源代码和编译产物
    ├── build/
    │   └── Debug/
    │       ├── libslvs.dll     # 动态链接库
    │       └── libslvs.lib     # 导入库
    └── libslvs/
        └── include/
            └── slvs.h          # C API头文件
```

## Mathor.pro 配置说明

### 关键配置项

1. **头文件包含路径**
   ```qmake
   INCLUDEPATH += $$PWD/SolveSpaceLib/libslvs/include
   ```

2. **定义共享库宏**
   ```qmake
   DEFINES += SLVS_LIB_SHARED
   ```

3. **链接库文件**
   - Debug模式: 链接 `Debug/libslvs.lib`
   - Release模式: 链接 `Release/libslvs.lib`

4. **自动复制DLL**
   - 编译后自动将 `libslvs.dll` 复制到输出目录
   - 确保程序运行时能找到DLL

## GeometrySolver 类

### 功能特性

`GeometrySolver` 是对 SolveSpaceLib 的Qt封装类，提供了：

- **Qt属性支持**: 可以在QML中直接访问
- **信号槽机制**: 异步通知求解结果
- **简化的API**: 隐藏底层复杂性

### C++ 使用示例

```cpp
#include "GeometrySolver.h"

// 创建求解器实例
GeometrySolver solver;

// 求解两点距离约束
// 参数: 点1坐标(x1,y1), 点2坐标(x2,y2), 目标距离
bool success = solver.solveSimple2DDistance(10, 20, 50, 60, 100.0);

if (success) {
    // 获取求解后的坐标
    QVariantMap result = solver.getSolvedPoints();
    double x1 = result["x1"].toDouble();
    double y1 = result["y1"].toDouble();
    double x2 = result["x2"].toDouble();
    double y2 = result["y2"].toDouble();
    
    // 获取自由度信息
    int dof = solver.dof();
}
```

### QML 使用示例

```qml
import Mathor.Solver 1.0

// 方式1: 创建求解器实例
GeometrySolver {
    id: solver
    
    onSolvingFinished: function(success) {
        if (success) {
            var points = solver.getSolvedPoints()
            console.log("点1:", points.x1, points.y1)
            console.log("点2:", points.x2, points.y2)
            console.log("自由度:", solver.dof)
        } else {
            console.log("错误:", solver.lastError)
        }
    }
}

// 方式2: 使用全局实例（在C++中通过setContextProperty注册）
Button {
    text: "求解"
    onClicked: {
        globalSolver.solveSimple2DDistance(10, 20, 50, 60, 100.0)
    }
}
```

## API 参考

### GeometrySolver 类

#### 属性 (Properties)

| 属性 | 类型 | 说明 |
|------|------|------|
| `dof` | int | 只读，系统的自由度数量 |
| `lastError` | QString | 只读，最后一次错误信息 |

#### 方法 (Methods)

**solveSimple2DDistance(x1, y1, x2, y2, targetDistance)**
- 求解两点距离约束
- 参数:
  - `x1, y1`: 第一个点的初始坐标
  - `x2, y2`: 第二个点的初始坐标
  - `targetDistance`: 目标距离
- 返回: `bool` - 求解是否成功

**getSolvedPoints()**
- 获取求解后的点坐标
- 返回: `QVariantMap` 包含 `x1`, `y1`, `x2`, `y2` 键值对

#### 信号 (Signals)

**solvingFinished(bool success)**
- 求解完成时发出
- 参数: `success` - 求解是否成功

**dofChanged()**
- 自由度改变时发出

**lastErrorChanged()**
- 错误信息改变时发出

## SolveSpaceLib 功能介绍

### 支持的几何元素

- **点** (Points): 2D点、3D点
- **线** (Lines): 线段、射线
- **圆** (Circles): 圆、圆弧
- **曲线** (Curves): 三次贝塞尔曲线
- **工作平面** (Workplanes): 2D约束的基准面

### 支持的约束类型

- **距离约束**: 点-点距离、点-线距离、点-面距离
- **角度约束**: 线-线角度
- **重合约束**: 点重合、点在线上、点在面上
- **平行/垂直**: 线平行、线垂直
- **对称约束**: 对称、水平对称、垂直对称
- **等长/等半径**: 线段等长、圆等半径
- **其他**: 中点约束、水平/垂直约束等

### 求解结果状态

| 状态 | 值 | 说明 |
|------|---|------|
| `SLVS_RESULT_OKAY` | 0 | 求解成功 |
| `SLVS_RESULT_INCONSISTENT` | 1 | 约束不一致（过约束） |
| `SLVS_RESULT_DIDNT_CONVERGE` | 2 | 未收敛 |
| `SLVS_RESULT_TOO_MANY_UNKNOWNS` | 3 | 未知数过多 |

## 编译和运行

### 前提条件

1. Qt 5.15.2 或更高版本
2. MSVC 2019 编译器（Windows）
3. SolveSpaceLib 已编译（`libslvs.dll` 存在于 `SolveSpaceLib/build/Debug/`）

### 编译步骤

1. 在Qt Creator中打开 `Mathor.pro`
2. 选择构建套件（Desktop Qt 5.15.2 MSVC2019 64bit）
3. 构建项目（Ctrl+B）

### 运行

- 点击运行按钮（Ctrl+R）
- 程序启动时会在控制台输出测试结果
- GUI界面提供交互式求解功能

## 扩展开发

### 添加更多约束类型

可以在 `GeometrySolver` 类中添加更多方法来支持其他约束类型：

```cpp
// 示例: 添加平行约束
bool solveParallelLines(double x1, double y1, double x2, double y2,
                        double x3, double y3, double x4, double y4);

// 示例: 添加圆半径约束
bool solveCircleRadius(double centerX, double centerY, 
                       double pointX, double pointY,
                       double targetRadius);
```

### 3D 求解

SolveSpaceLib 也支持3D几何求解，可以参考 `SolveSpaceLib/CDemo.c` 中的 `Example3d()` 函数。

### 复杂约束系统

对于复杂的多约束系统，可以：
1. 创建多个工作组 (Groups)
2. 添加多个实体 (Entities)
3. 定义多个约束 (Constraints)
4. 分组求解

参考 `SolveSpaceLib/CDemo.c` 中的 `Example2d()` 函数了解完整示例。

## 常见问题

### Q: 运行时找不到 libslvs.dll？

**A:** 确保 `libslvs.dll` 在以下位置之一：
- 可执行文件所在目录
- 系统PATH环境变量包含的目录

`.pro` 文件中的 `QMAKE_POST_LINK` 会自动复制DLL，如果失败，请手动复制。

### Q: 求解失败返回 INCONSISTENT？

**A:** 可能原因：
- 约束冲突（如同时要求距离为10和20）
- 过约束（约束数量超过必要）
- 初始值不合理

解决方法：
- 检查约束逻辑
- 减少约束数量
- 调整初始坐标

### Q: 如何调试求解问题？

**A:** 启用失败约束计算：
```cpp
m_sys.calculateFaileds = 1;
```

然后检查 `m_sys.failed[]` 数组，找出有问题的约束。

### Q: 性能优化？

**A:** 
- 合理设置初始值（接近最终解）
- 使用拖拽约束固定不需要移动的点
- 避免不必要的约束
- 分组求解复杂系统

## 参考资料

- [SolveSpace 官方网站](https://solvespace.com/)
- [SolveSpaceLib GitHub](https://github.com/solvespace/solvespace)
- `SolveSpaceLib/libslvs/include/slvs.h` - C API 文档
- `SolveSpaceLib/CDemo.c` - C 使用示例
- `SolveSpaceLib/example_S_Curve.cpp` - C++ 使用示例

## 许可证

- Mathor: 根据您的项目许可证
- SolveSpaceLib: GPLv3（参见 `SolveSpaceLib/LICENSE`）

## 联系方式

如有问题或建议，请通过项目Issue页面联系。

---

**祝开发顺利！** 🚀

