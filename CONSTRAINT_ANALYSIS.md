# 约束问题分析

## 当前问题

从调试输出可以看出：
```
GeometrySolver: Created point 2 at 171 171 with params 9 10
GeometrySolver: Setting drag constraints - fixed 2 parameters
GeometrySolver: Dragged point 2 is free to move
GeometrySolver: drag constraint solve successfully!
GeometrySolver: pt 1 after solve: ( 9.92341 ,  19.9234 )
GeometrySolver: pt 2 after solve: ( 171 ,  171 )
GeometrySolver: dof:  4
```

**关键问题**：自由度(DOF)是4，这意味着系统有4个自由度，约束没有起到限制作用。

## 问题分析

### 1. 自由度计算
- 2个点，每个点有2个参数（X, Y）= 4个参数
- 1个距离约束 = 1个约束方程
- 理论自由度 = 4 - 1 = 3

但实际自由度是4，说明约束没有被正确应用。

### 2. 可能的原因

#### 原因1：约束没有被添加
- 约束数据可能为空
- 约束添加逻辑可能有问题

#### 原因2：约束参数错误
- 工作平面ID可能错误
- 实体ID可能错误
- 约束类型可能错误

#### 原因3：拖拽约束设置错误
- 拖拽约束可能覆盖了距离约束
- 参数固定可能不正确

## 调试步骤

### 步骤1：检查约束数据
运行程序后，点击"测试约束"按钮，查看：
```
EaSession: Number of constraints: 1
EaSession: Constraint 0: QMap(("distance", QVariant(double, 100)) ...)
```

### 步骤2：检查约束添加
拖拽时查看约束添加的调试输出：
```
GeometrySolver: Adding constraints, total constraints to add: 1
GeometrySolver: Processing constraint type: distance
GeometrySolver: Distance constraint - point1Id: 1 point2Id: 2 distance: 100
GeometrySolver: Added distance constraint 1 between points 1 and 2
```

### 步骤3：检查系统参数
查看求解后的系统参数：
```
GeometrySolver: Total params: X entities: Y constraints: Z
```

## 可能的解决方案

### 方案1：检查约束数据传递
确保约束数据正确传递到GeometrySolver：
```cpp
// 在EaSession::solveDragConstraint中添加
qDebug() << "EaSession: Constraints being passed:" << m_constraints;
```

### 方案2：验证工作平面设置
检查工作平面ID是否正确：
```cpp
// 当前使用工作平面ID 200
m_sys.constraint[m_sys.constraints++] = Slvs_MakeConstraint(
    constraintId, g,
    SLVS_C_PT_PT_DISTANCE,
    200,  // 工作平面ID
    distance,
    pointToEntity[point1Id], pointToEntity[point2Id], 0, 0);
```

### 方案3：简化测试
创建一个最简单的测试，只包含距离约束：
```cpp
// 测试：固定点1，让点2在距离约束下移动
// 不设置拖拽约束，只设置距离约束
```

## 预期结果

如果约束正确工作，应该看到：
- 自由度 = 1（2个点4个参数 - 1个距离约束 - 2个固定参数 = 1）
- 点2沿着以点1为圆心、半径为100的圆弧移动

## 下一步行动

1. 运行程序，点击"测试约束"按钮
2. 拖拽点2，观察约束添加的调试输出
3. 根据输出结果确定问题所在
4. 应用相应的修复方案

关键是要确认约束是否被正确添加到SolveSpaceLib系统中。
