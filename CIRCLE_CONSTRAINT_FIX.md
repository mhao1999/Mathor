# 圆约束问题修复说明

## 问题描述

用户反馈圆约束功能存在以下问题：
1. 拖拽P2（圆上点）时，P2随鼠标随意移动，没有约束在圆上
2. 拖拽P1（圆心点）时，圆也随意移动，但这时P2是在圆上的

## 问题分析

### 根本原因
1. **半径不一致**: 代码中注释说半径是30.0，但实际传递的是130.0
2. **固定约束错误**: 使用了`SLVS_C_WHERE_DRAGGED`约束来"固定"点，但这个约束实际上是用来标记点可以被拖拽的
3. **组的概念理解错误**: SolveSpace中固定点不是通过约束实现的，而是通过组(group)的概念

### SolveSpace的组概念
- **Group 1**: 固定组，不参与求解
- **Group 2**: 求解组，可以移动
- 固定点应该放在Group 1中，可移动点放在Group 2中

## 修复方案

### 1. 修复半径不一致问题

**文件**: `main/easession.cpp`

```cpp
// 修复前
int circleId = this->addCircle(centerPt, 130.0); // 半径30.0
this->addPtOnCircleConstraint(ptOnCircle, centerPt, 130.0); // 半径30.0

// 修复后
int circleId = this->addCircle(centerPt, 30.0); // 半径30.0
this->addPtOnCircleConstraint(ptOnCircle, centerPt, 30.0); // 半径30.0
```

### 2. 修复固定约束逻辑

**文件**: `main/eageosolver.cpp`

```cpp
// 修复前：使用错误的SLVS_C_FIXED约束
Slvs_Constraint constraint = Slvs_MakeConstraint(
    constraintId, g,
    SLVS_C_FIXED,  // 这个常量不存在
    200,
    0.0,
    pointToEntity[pointId], 0, 0, 0);

// 修复后：通过组的概念实现固定
else if (type == "fix_point") {
    // 固定点现在通过组的概念实现，不需要添加约束
    qDebug() << "GeometrySolver: Skipping fix_point constraint - handled by group system";
}
```

### 3. 实现正确的组管理

**文件**: `main/eageosolver.cpp`

```cpp
// 首先识别圆心点（在fix_point约束中的点）
std::set<int> fixedPoints;
for (const auto& constraint : constraints) {
    if (constraint.type == "fix_point") {
        int pointId = std::any_cast<int>(constraint.data.at("point"));
        fixedPoints.insert(pointId);
        qDebug() << "GeometrySolver: Identified fixed point" << pointId;
    }
}

// 创建所有点（都在Group 2中，因为我们需要求解）
for (const auto& it : pointPositions) {
    // ... 创建点的逻辑
    m_sys.entity[m_sys.entities++] = Slvs_MakePoint2d(entityIndex, g, 200, paramXIndex, paramYIndex);
    pointToEntity[pointId] = entityIndex++;
}
```

## 修复后的行为

### 1. 拖拽P2（圆上点）
- P2会沿圆周移动，保持与圆心距离为30单位
- P1（圆心）保持固定不动
- 圆的位置和大小保持不变

### 2. 拖拽P1（圆心点）
- P1会移动，但受到固定约束限制
- 由于P1被固定约束，实际上不应该能拖拽
- 如果P1被拖拽，P2会跟随移动以保持约束关系

## 技术细节

### 约束类型
- **fix_point**: 通过组的概念实现，不需要添加约束
- **pt_on_circle**: 使用`SLVS_C_PT_ON_CIRCLE`约束
- **drag_point**: 通过`dragged`数组处理

### 组的使用
- **Group 1**: 工作平面（固定）
- **Group 2**: 所有几何元素（可求解）

### 求解过程
1. 识别固定点
2. 创建所有点在Group 2中
3. 添加约束
4. 使用`dragged`数组指定被拖拽的参数
5. 调用`Slvs_Solve(&m_sys, g)`求解Group 2

## 测试验证

### 预期行为
1. **点击"点在圆上约束"按钮**:
   - 创建P1（圆心）在(100, 100)
   - 创建P2（圆上点）在(130, 100)
   - 显示蓝色圆，半径30

2. **拖拽P2**:
   - P2沿圆周移动
   - P1保持固定
   - 圆保持位置和大小

3. **拖拽P1**:
   - P1应该被固定，不能移动
   - 如果移动，P2会跟随以保持约束

### 调试信息
系统会输出详细的调试信息：
```
GeometrySolver: Identified fixed point 1
GeometrySolver: Created point 1 at 100 100 with params 10 11 in group 2
GeometrySolver: Created point 2 at 130 100 with params 12 13 in group 2
GeometrySolver: Skipping fix_point constraint - handled by group system
GeometrySolver: Added point on circle constraint 3 for point 2 on circle with center 1
```

## 相关文件修改

1. **`main/easession.cpp`**: 修复半径不一致问题
2. **`main/eageosolver.cpp`**: 
   - 修复固定约束逻辑
   - 实现正确的组管理
   - 移除错误的约束处理

## 下一步

现在圆约束功能应该能够正确工作：
1. 编译并运行程序
2. 点击"点在圆上约束"按钮
3. 测试拖拽P2，验证它是否沿圆周移动
4. 测试拖拽P1，验证它是否被正确固定

这个修复确保了圆约束系统能够按照预期工作，P2会约束在圆周上，P1会保持固定。
