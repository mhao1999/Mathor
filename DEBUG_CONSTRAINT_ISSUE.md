# 约束问题诊断指南

## 当前问题
约束数量为0，导致拖拽时没有约束生效。

## 调试步骤

### 步骤1：确认点存在
1. 运行程序
2. 点击"示例 1: 两点距离约束"按钮
3. 观察调试输出，确认点1和点2是否被正确添加

**预期输出**：
```
EaSession: Added point 1 at 10 20
EaSession: Added point 2 at 50 60
```

### 步骤2：检查约束添加
在步骤1之后，观察约束添加的调试输出：

**预期输出**：
```
EaSession: addDistanceConstraint called for points 1 2 distance 100
EaSession: Current number of points: 2
EaSession: Current number of constraints: 0
EaSession: Available point ID: 1 at 10 20
EaSession: Available point ID: 2 at 50 60
EaSession: Point1 exists: true Point2 exists: true
EaSession: Added distance constraint 1 between points 1 and 2 with distance 100
EaSession: Total constraints after adding: 1
EaSession: Constraint details: QMap(("distance", QVariant(double, 100))("id", QVariant(int, 1))("point1", QVariant(int, 1))("point2", QVariant(int, 2))("type", QVariant(QString, "distance")))
```

### 步骤3：测试手动添加约束
1. 点击"手动添加约束"按钮
2. 观察是否成功添加约束

### 步骤4：测试约束查询
1. 点击"测试约束"按钮
2. 观察约束数量是否正确

## 可能的问题和解决方案

### 问题1：点不存在
**症状**：看到"Point1 exists: false"或"Point2 exists: false"
**原因**：点没有被正确添加或ID不匹配
**解决方案**：
- 检查addPoint方法是否正确设置ID
- 确认点的ID是1和2

### 问题2：约束添加失败
**症状**：约束添加方法被调用但约束数量仍为0
**原因**：约束添加逻辑有问题
**解决方案**：
- 检查QVariantMap的创建
- 检查m_constraints.append()调用

### 问题3：约束被清空
**症状**：约束添加成功但后续查询时数量为0
**原因**：某个地方调用了clear()方法
**解决方案**：
- 检查是否有意外的clear()调用
- 添加clear()调用的调试信息

### 问题4：时序问题
**症状**：约束添加时点不存在
**原因**：约束添加的时机不对
**解决方案**：
- 确保在添加约束前点已经存在
- 使用Timer延迟添加约束

## 测试用例

### 测试用例1：基本约束添加
```qml
// 1. 清空
globalSession.clear()

// 2. 添加点
globalSession.addPoint(10, 20)  // 点1
globalSession.addPoint(50, 60)  // 点2

// 3. 添加约束
globalSession.addDistanceConstraint(1, 2, 100.0)

// 4. 验证
globalSession.testConstraints()
```

### 测试用例2：手动约束添加
```qml
// 点击"手动添加约束"按钮
// 观察调试输出
```

### 测试用例3：拖拽测试
```qml
// 1. 创建测试场景
globalSession.clear()
globalSession.addPoint(10, 20)
globalSession.addPoint(50, 60)
globalSession.addDistanceConstraint(1, 2, 100.0)

// 2. 测试拖拽
globalSession.testDragConstraint(2, 100, 100)
```

## 预期结果

如果一切正常，应该看到：
1. 点1和点2被正确添加
2. 约束被成功添加，数量为1
3. 拖拽时约束生效，DOF < 4
4. 点2沿着以点1为圆心的圆弧移动

## 下一步

根据调试输出确定具体问题：
- 如果点不存在 → 修复点添加逻辑
- 如果约束添加失败 → 修复约束添加逻辑
- 如果约束被清空 → 找到清空的位置
- 如果时序问题 → 调整调用顺序
