# 拖拽约束功能使用说明

## 概述
现在系统支持拖拽约束功能，当您拖拽一个点时，系统会自动考虑几何约束，确保被拖拽的点在约束允许的范围内移动。

## 功能特性

### 1. 约束感知的拖拽
- 当您拖拽点2时，由于存在100.0的距离约束，点2会在以点1为圆心的圆上移动
- 系统使用SolveSpaceLib进行约束求解，确保几何关系的一致性

### 2. 约束管理
- 支持距离约束的添加和管理
- 约束与几何元素（点、线）关联
- 自动验证约束的有效性

## 使用方法

### 1. 创建带约束的几何图形
```qml
// 添加两个点
globalSession.addPoint(10, 20)
globalSession.addPoint(50, 60)

// 添加连接线
globalSession.addLine(1, 2)

// 添加距离约束（100.0单位）
globalSession.addDistanceConstraint(1, 2, 100.0)
```

### 2. 拖拽操作
1. **选择点**：点击要拖拽的点
2. **拖拽**：按住鼠标左键并移动鼠标
3. **约束应用**：系统自动应用约束，确保点2在距离点1为100.0的圆上移动

### 3. 约束管理
```qml
// 获取所有约束
var constraints = globalSession.getConstraints()

// 清除所有约束
globalSession.clearConstraints()

// 移除特定约束
globalSession.removeConstraint(constraintId)
```

## 技术实现

### 1. 架构层次
```
EaDrawingArea (视图层)
    ↓ 鼠标事件
EaPoint::onDragWithConstraints (几何体层)
    ↓ 约束求解
EaSession::solveDragConstraint (会话管理层)
    ↓ 调用求解器
GeometrySolver::solveDragConstraint (求解器层)
```

### 2. 约束求解流程
1. **收集几何信息**：获取所有点的当前位置
2. **构建约束系统**：将约束转换为SolveSpaceLib格式
3. **设置拖拽参数**：指定被拖拽点的目标位置
4. **求解**：调用SolveSpaceLib进行约束求解
5. **更新位置**：将求解结果应用到几何元素

### 3. 关键类和方法

#### EaPoint
- `onDragWithConstraints(x, y, session)` - 约束感知的拖拽方法
- `onDrag(x, y)` - 简单拖拽方法（无约束）

#### EaSession
- `addDistanceConstraint(point1Id, point2Id, distance)` - 添加距离约束
- `solveDragConstraint(draggedPointId, newX, newY)` - 拖拽约束求解
- `setGeometrySolver(solver)` - 设置几何求解器引用

#### GeometrySolver
- `solveDragConstraint(draggedPointId, newX, newY, pointPositions, constraints)` - 核心约束求解

## 示例场景

### 场景1：两点距离约束
1. 创建点1在(10, 20)
2. 创建点2在(50, 60)
3. 添加距离约束100.0
4. 拖拽点2时，点2会在以点1为圆心、半径为100.0的圆上移动

### 场景2：三角形约束
1. 创建三个点
2. 添加三条边的距离约束
3. 拖拽任意一个点时，其他点会根据约束自动调整

## 注意事项

1. **约束一致性**：确保约束系统不会过约束或产生矛盾
2. **求解失败处理**：如果约束求解失败，系统会回退到简单拖拽模式
3. **性能考虑**：约束求解需要计算时间，复杂约束系统可能影响拖拽响应性
4. **约束验证**：添加约束时会验证相关点是否存在

## 调试信息

系统会输出详细的调试信息：
- 约束添加/移除
- 拖拽操作开始/结束
- 约束求解成功/失败
- 几何元素位置更新

查看控制台输出可以了解约束系统的运行状态。
