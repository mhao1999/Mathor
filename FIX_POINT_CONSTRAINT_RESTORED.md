# 固定点约束恢复说明

## 用户请求

用户要求将`fix_point`约束恢复为使用`SLVS_C_WHERE_DRAGGED`表示，而不是使用组的概念。

## 修复内容

### 1. 恢复fix_point约束处理

**文件**: `main/eageosolver.cpp`

```cpp
// 修复前：跳过fix_point约束
else if (type == "fix_point") {
    // 固定点现在通过组的概念实现，不需要添加约束
    qDebug() << "GeometrySolver: Skipping fix_point constraint - handled by group system";
}

// 修复后：使用SLVS_C_WHERE_DRAGGED约束
else if (type == "fix_point") {
    int pointId = std::any_cast<int>(constraint.data.at("point"));
    
    if (pointToEntity.find(pointId) != pointToEntity.end()) {
        // 添加固定点约束 - 使用WHERE_DRAGGED约束
        int constraintId = m_sys.constraints + 1;
        Slvs_Constraint constraint = Slvs_MakeConstraint(
            constraintId, g,
            SLVS_C_WHERE_DRAGGED,
            200,
            0.0,
            pointToEntity[pointId], 0, 0, 0);
        constraint.entityC = 0;
        constraint.entityD = 0;
        m_sys.constraint[m_sys.constraints++] = constraint;
        
        qDebug() << "GeometrySolver: Added fix point constraint" << constraintId
                 << "for point" << pointId << "entity" << pointToEntity[pointId];
    } else {
        qWarning() << "GeometrySolver: Cannot add fix point constraint - point" << pointId << "not found";
    }
}
```

### 2. 移除组识别逻辑

**文件**: `main/eageosolver.cpp`

```cpp
// 移除了以下代码：
// 首先识别圆心点（在fix_point约束中的点）
std::set<int> fixedPoints;
for (const auto& constraint : constraints) {
    if (constraint.type == "fix_point") {
        int pointId = std::any_cast<int>(constraint.data.at("point"));
        fixedPoints.insert(pointId);
        qDebug() << "GeometrySolver: Identified fixed point" << pointId;
    }
}
```

### 3. 恢复centerToCircleEntity映射

**文件**: `main/eageosolver.cpp`

```cpp
// 恢复被注释的映射声明
std::map<int, int> centerToCircleEntity; // 圆心点ID到圆实体ID的映射
```

### 4. 修复createFixPointConstraint调用

**文件**: `main/easession.cpp`

```cpp
// 修复前：错误的参数类型
this->createFixPointConstraint(circleId); // circleId是圆ID，不是点ID

// 修复后：正确的参数类型
this->createFixPointConstraint(centerPt); // centerPt是点ID
```

## 约束系统说明

### SLVS_C_WHERE_DRAGGED的作用

在SolveSpace中，`SLVS_C_WHERE_DRAGGED`约束的作用是：
- 标记一个点可以被拖拽
- 当求解器需要调整参数时，会优先保持被标记为`WHERE_DRAGGED`的点不变
- 其他点会移动以满足约束条件

### 固定点的实现方式

通过`SLVS_C_WHERE_DRAGGED`约束实现固定点：
1. 为需要固定的点添加`SLVS_C_WHERE_DRAGGED`约束
2. 在拖拽时，该点会保持其位置不变
3. 其他点会移动以满足约束条件

## 预期行为

### 拖拽P2（圆上点）
- P2会沿圆周移动，保持与圆心距离30单位
- P1（圆心）由于有`SLVS_C_WHERE_DRAGGED`约束，会保持固定
- 圆的位置和大小保持不变

### 拖拽P1（圆心点）
- P1由于有`SLVS_C_WHERE_DRAGGED`约束，会保持固定
- 如果强制拖拽P1，P2会跟随移动以保持约束关系

## 调试信息

系统会输出详细的调试信息：
```
GeometrySolver: Added fix point constraint 1 for point 1 entity 300
GeometrySolver: Added point on circle constraint 2 for point 2 on circle with center 1
```

## 相关文件修改

1. **`main/eageosolver.cpp`**: 
   - 恢复`fix_point`约束处理
   - 移除组识别逻辑
   - 恢复`centerToCircleEntity`映射

2. **`main/easession.cpp`**: 
   - 修复`createFixPointConstraint`调用参数

## 技术细节

### 约束创建过程
1. 创建所有点和线段实体
2. 创建圆实体（为点在圆上约束）
3. 添加`fix_point`约束（使用`SLVS_C_WHERE_DRAGGED`）
4. 添加`pt_on_circle`约束（使用`SLVS_C_PT_ON_CIRCLE`）
5. 使用`dragged`数组指定被拖拽的参数
6. 调用求解器

### 约束ID管理
- 使用`m_sys.constraints + 1`作为约束ID
- 确保约束ID的唯一性

现在固定点约束已经恢复为使用`SLVS_C_WHERE_DRAGGED`的方式，应该能够正确工作。
