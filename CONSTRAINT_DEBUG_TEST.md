# 约束调试测试指南

## 问题描述
点2能移动了，但是距离约束不起作用，点2只是简单地跟随鼠标移动。

## 调试步骤

### 1. 编译并运行程序
```bash
cd D:\qt\Mathor
qmake
nmake
# 运行生成的Mathor.exe
```

### 2. 创建测试场景
1. 点击"添加点1"按钮
2. 点击"添加点2"按钮  
3. 点击"添加线段"按钮（会自动添加距离约束）

### 3. 观察控制台输出
在添加约束时，应该看到：
```
EaSession: Added distance constraint 1 between points 1 and 2 with distance 100
```

### 4. 测试拖拽
1. 点击点2开始拖拽
2. 移动鼠标
3. 观察控制台输出

### 5. 预期的调试输出序列
```
EaPoint: onDragWithConstraints called for point 2 to position [x] [y]
EaSession: solveDragConstraint called for point 2 to position [x] [y]
EaSession: Number of constraints: 1
EaSession: Constraint 0: QMap(("distance", QVariant(double, 100)) ("id", QVariant(int, 1)) ("point1", QVariant(int, 1)) ("point2", QVariant(int, 2)) ("type", QVariant(QString, "distance")))
EaSession: Point 1 at QMap(("x", QVariant(double, 10)) ("y", QVariant(double, 20)) ("z", QVariant(double, 0)))
EaSession: Point 2 at QMap(("x", QVariant(double, 50)) ("y", QVariant(double, 60)) ("z", QVariant(double, 0)))
GeometrySolver: solveDragConstraint called for point 2 to position [x] [y]
GeometrySolver: Using new position for dragged point 2: [x] [y]
GeometrySolver: Created point 1 at 10 20 with params [paramX] [paramY]
GeometrySolver: Created point 2 at [x] [y] with params [paramX] [paramY]
GeometrySolver: Added distance constraint between points 1 and 2 distance: 100
GeometrySolver: Setting drag constraints for point 2 params: [paramX] [paramY]
GeometrySolver: 拖拽约束求解成功!
GeometrySolver: 求解后点1: ([x1], [y1])
GeometrySolver: 求解后点2: ([x2], [y2])
EaPoint: Drag with constraints successful for point 2
```

### 6. 如果约束求解失败
如果看到：
```
EaPoint: Constraint solving failed, using simple drag for point 2
```

这说明约束求解失败了，需要检查：
1. GeometrySolver是否正确设置
2. 约束数据是否正确
3. SolveSpaceLib是否正确工作

## 可能的问题和解决方案

### 问题1：没有约束
**症状**：控制台显示 "Number of constraints: 0"
**原因**：约束没有被正确添加
**解决方案**：检查QML中的约束添加调用

### 问题2：GeometrySolver未设置
**症状**：控制台显示 "No GeometrySolver available for constraint solving"
**原因**：EaSession没有GeometrySolver引用
**解决方案**：检查main.cpp中的setGeometrySolver调用

### 问题3：约束求解失败
**症状**：控制台显示 "拖拽约束求解失败"
**原因**：SolveSpaceLib求解失败
**解决方案**：检查约束系统是否一致

### 问题4：参数映射错误
**症状**：约束求解成功但位置不正确
**原因**：参数索引映射错误
**解决方案**：检查GeometrySolver中的参数映射逻辑

## 快速测试方法

### 测试1：验证约束添加
在QML中添加一个测试按钮：
```qml
Button {
    text: "测试约束"
    onClicked: {
        console.log("约束数量:", globalSession.getConstraints().length)
        var constraints = globalSession.getConstraints()
        for (var i = 0; i < constraints.length; i++) {
            console.log("约束", i, ":", constraints[i])
        }
    }
}
```

### 测试2：验证GeometrySolver
在QML中添加一个测试按钮：
```qml
Button {
    text: "测试求解器"
    onClicked: {
        // 尝试手动调用约束求解
        var success = globalSession.solveDragConstraint(2, 100, 100)
        console.log("约束求解结果:", success)
    }
}
```

## 预期结果

如果一切正常，拖拽点2时应该看到：
1. 点2沿着以点1为圆心、半径为100的圆弧移动
2. 线段长度始终保持100
3. 控制台显示约束求解成功的消息

如果点2只是跟随鼠标移动，说明约束求解失败，回退到了简单拖拽模式。
