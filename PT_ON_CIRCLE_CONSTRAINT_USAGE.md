# 点在圆上约束功能使用说明

## 功能概述

点在圆上约束功能允许用户创建一个点，并确保该点始终位于指定的圆上。当用户拖拽该点时，点会自动调整位置以保持在圆上。

## 使用方法

### 1. 创建点在圆上约束场景

1. 点击"点在圆上约束"按钮
2. 系统会自动创建：
   - 2个点：圆心点(100,100)、圆上点(130,100)
   - 1个圆：以圆心为中心，半径30.0
   - 1个固定约束：圆心被固定
   - 1个点在圆上约束：圆上点位于圆上

### 2. 测试拖拽功能

1. 点击"测试拖拽约束"按钮
2. 系统会尝试将圆上点拖拽到新位置
3. 求解器会自动调整圆上点的位置以保持在圆上

### 3. 手动拖拽测试

1. 在绘图区域中直接拖拽圆上点
2. 观察圆上点如何自动调整以保持在圆上

## 技术实现

### 约束类型

- **SLVS_C_WHERE_DRAGGED**: 固定圆心位置
- **SLVS_C_PT_ON_CIRCLE**: 使点位于圆上

### 实体创建

1. **圆心点**: 使用`Slvs_MakePoint2d`创建
2. **半径距离实体**: 使用`Slvs_MakeDistance`创建
3. **圆实体**: 使用`Slvs_MakeCircle`创建，需要圆心、法向量和半径

### 求解过程

1. 当用户拖拽圆上点时，系统调用`solveDragConstraint`方法
2. 求解器创建SolveSpace系统，包含所有点、圆和约束
3. 应用所有约束条件
4. 求解器计算新的点位置
5. 更新界面显示

### 关键代码位置

- `EaSession::createPtOnCircleConstraint()`: 创建点在圆上约束场景
- `EaSession::addPtOnCircleConstraint()`: 添加点在圆上约束
- `GeometrySolver::solveDragConstraint()`: 求解拖拽约束

## 预期效果

当拖拽圆上点时：
- 圆心保持固定位置
- 圆上点自动调整位置以保持在圆上
- 圆的半径保持不变
- 所有约束条件得到满足

## 系统自由度分析

- 总自由度：4（2个点的X、Y坐标）
- 固定约束：2个（圆心的X、Y参数）
- 点在圆上约束：1个
- 剩余自由度：4 - 2 - 1 = 1

这意味着圆上点可以沿圆周移动，但必须保持在圆上。

## 调试信息

系统会在控制台输出详细的调试信息，包括：
- 创建的点、圆和约束
- 求解过程中的参数和实体
- 求解结果和自由度信息

### 预期调试输出
```
EaSession: Created point on circle constraint with center point 1 and point on circle 2 with radius 30.0
GeometrySolver: Created point 1 at 100 100 with params 10 11
GeometrySolver: Created point 2 at 130 100 with params 12 13
GeometrySolver: Created circle with center point 1 radius 30.0 entity 304
GeometrySolver: Added point on circle constraint 1 for point 2 on circle 1 entities 303 304
```

## 注意事项

1. 确保在创建约束前先点击"点在圆上约束"按钮
2. 如果求解失败，检查控制台输出的错误信息
3. 点在圆上约束需要至少一个点和一个圆才能生效
4. 圆心应该被固定以确保约束系统稳定
5. 圆的半径是固定的，不能通过拖拽改变

## 扩展功能

这个功能可以扩展为：
- 多个点在同一个圆上
- 点在多个圆的交点上
- 点在圆上的特定位置（如0度、90度等）
- 点在圆上的相对位置约束
- 动态半径的圆（通过其他约束控制半径）

## 与其他约束的结合

点在圆上约束可以与其他约束结合使用：
- 与平行约束结合：创建与圆相切的直线
- 与距离约束结合：控制圆上点之间的距离
- 与角度约束结合：控制圆上点之间的角度关系
