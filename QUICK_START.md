# 快速开始指南

## 🎯 集成完成！

SolveSpaceLib 已成功集成到 Mathor 项目中。

## 📝 已完成的修改

### 1. Mathor.pro 配置
- ✅ 添加了 SolveSpaceLib 头文件路径
- ✅ 配置了动态库链接
- ✅ 添加了自动复制 DLL 的脚本
- ✅ 添加了新的源文件

### 2. 创建的新文件
- ✅ `GeometrySolver.h` - Qt封装类头文件
- ✅ `GeometrySolver.cpp` - Qt封装类实现
- ✅ 更新 `main.cpp` - 添加了测试代码和QML注册
- ✅ 更新 `main.qml` - 添加了演示UI

## 🚀 立即开始

### 步骤 1: 打开项目
在 Qt Creator 中打开 `Mathor.pro`

### 步骤 2: 构建项目
按 `Ctrl+B` 或点击"构建"按钮

### 步骤 3: 运行
按 `Ctrl+R` 或点击"运行"按钮

### 步骤 4: 查看结果

#### 控制台输出
程序启动时会在控制台显示测试结果：
```
===============================================
测试 SolveSpaceLib 集成
===============================================
求解问题: 两点距离约束
初始点1: (10, 20)
初始点2: (50, 60)
目标距离: 100.0
-----------------------------------------------
求解成功!
点1: ( 10 ,  20 )
点2: ( ... , ... )
自由度(DOF): 2
===============================================
✓ SolveSpaceLib 集成成功!
===============================================
```

#### GUI界面
应用程序窗口会显示：
- 输入参数面板（可以修改坐标和目标距离）
- 求解按钮
- 结果显示区域
- 可视化画布（显示点和连线）

## 💡 使用示例

### 在C++中使用

```cpp
#include "GeometrySolver.h"

GeometrySolver solver;
bool success = solver.solveSimple2DDistance(10, 20, 50, 60, 100.0);

if (success) {
    QVariantMap points = solver.getSolvedPoints();
    qDebug() << "X1:" << points["x1"].toDouble();
    qDebug() << "Y1:" << points["y1"].toDouble();
}
```

### 在QML中使用

```qml
import Mathor.Solver 1.0

GeometrySolver {
    id: solver
    onSolvingFinished: function(success) {
        if (success) {
            console.log("求解成功!")
        }
    }
}

Button {
    text: "求解"
    onClicked: {
        solver.solveSimple2DDistance(10, 20, 50, 60, 100.0)
    }
}
```

## 📚 详细文档

查看 `INTEGRATION_README.md` 了解：
- 完整的 API 参考
- SolveSpaceLib 功能介绍
- 扩展开发指南
- 常见问题解答

## ⚙️ 配置说明

### Mathor.pro 关键配置

```qmake
# 头文件路径
INCLUDEPATH += $$PWD/SolveSpaceLib/libslvs/include

# 定义共享库
DEFINES += SLVS_LIB_SHARED

# Debug模式链接
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/SolveSpaceLib/build/Debug -llibslvs
}
```

### 文件结构
```
Mathor/
├── GeometrySolver.h          ← Qt封装类
├── GeometrySolver.cpp         ← Qt封装实现
├── main.cpp                   ← 包含测试代码
├── main.qml                   ← UI界面
├── Mathor.pro                 ← 项目配置
└── SolveSpaceLib/
    └── build/Debug/
        ├── libslvs.dll        ← 动态库
        └── libslvs.lib        ← 导入库
```

## 🔧 故障排除

### 问题：找不到 libslvs.dll

**解决方案：**
1. 确认 `SolveSpaceLib/build/Debug/libslvs.dll` 存在
2. 重新构建项目（qmake 会自动复制）
3. 或手动复制到 `build/Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug/`

### 问题：链接错误

**解决方案：**
1. 确认 `SolveSpaceLib/build/Debug/libslvs.lib` 存在
2. 检查 Mathor.pro 中的路径是否正确
3. 确保使用的是 Debug 构建模式

### 问题：编译错误 - 找不到 slvs.h

**解决方案：**
1. 检查 `SolveSpaceLib/libslvs/include/slvs.h` 是否存在
2. 确认 Mathor.pro 中 INCLUDEPATH 正确

## 📊 示例场景

### 场景1: 固定距离的两点
```cpp
// 点1在(0,0)，点2移动到距离点1为50单位的位置
solver.solveSimple2DDistance(0, 0, 30, 40, 50.0);
```

### 场景2: 调整现有几何
```cpp
// 已有两点在(100,100)和(200,150)
// 调整它们的距离为80单位
solver.solveSimple2DDistance(100, 100, 200, 150, 80.0);
```

## 🎨 下一步

### 添加更多功能

1. **平行线约束**
   - 在 GeometrySolver 中添加新方法
   - 使用 `SLVS_C_PARALLEL` 约束

2. **垂直约束**
   - 使用 `SLVS_C_PERPENDICULAR` 约束

3. **圆和圆弧**
   - 使用 `Slvs_MakeCircle()` 和 `Slvs_MakeArcOfCircle()`

4. **3D 几何**
   - 参考 CDemo.c 中的 Example3d()

### 学习资源

- 阅读 `SolveSpaceLib/CDemo.c` 了解更多示例
- 查看 `slvs.h` 了解完整 API
- 参考 SolveSpace 官方文档

## ✅ 验证清单

- [ ] 项目成功编译
- [ ] 程序成功运行
- [ ] 控制台显示"✓ SolveSpaceLib 集成成功!"
- [ ] GUI界面正常显示
- [ ] 点击"求解约束"按钮能看到结果
- [ ] 可视化画布显示点和连线

## 🎉 恭喜！

你已经成功集成了 SolveSpaceLib 几何约束求解器！

现在你可以：
- ✨ 使用强大的几何约束求解功能
- 🎯 在 Qt/QML 应用中创建参数化几何
- 🚀 开发 CAD、绘图或几何建模应用

祝开发愉快！ 💪

