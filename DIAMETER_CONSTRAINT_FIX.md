# 直径约束修复说明

## 问题描述

用户反馈拖拽P2时，该点不是严格在圆弧上运动，但是有阻力阻止它向圆心靠近或往外拉。用户的需求是P2就在圆周的弧上运动。

## 问题分析

### 根本原因
我们只添加了`SLVS_C_PT_ON_CIRCLE`约束，但没有添加`SLVS_C_DIAMETER`约束来固定圆的半径。这导致：
1. 圆的大小可以变化
2. P2不能严格在圆周上运动
3. 拖拽时有阻力，因为圆的大小在调整

### 约束系统分析
- **SLVS_C_PT_ON_CIRCLE**: 约束点在圆上，但圆的大小可以变化
- **SLVS_C_DIAMETER**: 约束圆的直径（半径），固定圆的大小
- **SLVS_C_WHERE_DRAGGED**: 固定圆心位置

## 修复方案

### 添加直径约束

**文件**: `main/eageosolver.cpp`

```cpp
// 修复前：只创建圆实体，没有直径约束
// 使用圆心点ID作为键
centerToCircleEntity[centerPointId] = circleEntityIndex;

qDebug() << "GeometrySolver: Created circle with center point" << centerPointId 
         << "radius" << radius << "entity" << circleEntityIndex;

// 修复后：添加直径约束来固定圆的半径
// 使用圆心点ID作为键
centerToCircleEntity[centerPointId] = circleEntityIndex;

// 添加直径约束来固定圆的半径
int diameterConstraintId = m_sys.constraints + 1;
Slvs_Constraint diameterConstraint = Slvs_MakeConstraint(
    diameterConstraintId, g,
    SLVS_C_DIAMETER,
    200,
    radius * 2.0,  // 直径 = 半径 * 2
    circleEntityIndex, 0, 0, 0);
diameterConstraint.entityC = 0;
diameterConstraint.entityD = 0;
m_sys.constraint[m_sys.constraints++] = diameterConstraint;

qDebug() << "GeometrySolver: Created circle with center point" << centerPointId 
         << "radius" << radius << "entity" << circleEntityIndex;
qDebug() << "GeometrySolver: Added diameter constraint" << diameterConstraintId 
         << "for circle" << circleEntityIndex << "diameter" << (radius * 2.0);
```

## 约束系统说明

### 完整的圆约束系统
1. **SLVS_C_WHERE_DRAGGED**: 固定圆心位置
2. **SLVS_C_DIAMETER**: 固定圆的直径（半径）
3. **SLVS_C_PT_ON_CIRCLE**: 约束点在圆上

### 约束关系
- 圆心被固定，不能移动
- 圆的半径被固定，不能变化
- P2被约束在圆周上，可以沿圆周移动

## 预期行为

### 拖拽P2（圆上点）
- ✅ P2会严格沿圆周移动，保持与圆心距离30单位
- ✅ 没有阻力，因为圆的大小是固定的
- ✅ P1（圆心）保持固定不动
- ✅ 圆的位置和大小保持不变

### 拖拽P1（圆心点）
- ✅ P1被固定约束，不能移动
- ✅ 如果强制拖拽P1，P2会跟随移动以保持约束关系

## 调试信息

系统会输出详细的调试信息：
```
GeometrySolver: Created circle with center point 1 radius 30 entity 303
GeometrySolver: Added diameter constraint 1 for circle 303 diameter 60
GeometrySolver: Added fix point constraint 2 for point 1 entity 300
GeometrySolver: Added point on circle constraint 3 for point 2 on circle with center 1 entities 301 303
GeometrySolver: Total constraints added: 3
```

## 技术细节

### 直径约束参数
- **constraintId**: 约束ID
- **g**: 组ID（Group 2）
- **SLVS_C_DIAMETER**: 直径约束类型
- **200**: 工作平面ID
- **radius * 2.0**: 直径值（半径 * 2）
- **circleEntityIndex**: 圆实体ID

### 约束创建顺序
1. 创建圆实体
2. 添加直径约束（固定圆的大小）
3. 添加固定点约束（固定圆心）
4. 添加点在圆上约束（约束P2在圆周上）

## 相关文件修改

1. **`main/eageosolver.cpp`**: 在圆创建后添加直径约束

## 测试验证

现在圆约束功能应该能够正确工作：
1. P2会严格在圆周上移动
2. 没有阻力，因为圆的大小是固定的
3. 约束求解会正确工作
4. 圆的位置和大小保持不变

这个修复确保了圆的大小被固定，P2能够严格在圆周上运动，没有阻力。
