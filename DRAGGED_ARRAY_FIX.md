# 拖拽数组修复说明

## 问题描述

用户反馈拖拽P2时，它随鼠标随意移动，没有约束在圆上。从调试信息可以看出：

```
GeometrySolver: Added point on circle constraint 2 for point 2 on circle with center 1 entities 301 303
GeometrySolver: drag constraint solve successfully!
GeometrySolver: pt 1 after solve: ( 88 ,  118 )
GeometrySolver: pt 2 after solve: ( 112.984 ,  149.979 )
```

虽然约束被正确添加了，但是P2仍然随鼠标移动，没有约束在圆上。

## 问题分析

### 根本原因
1. **centerToCircleEntity映射被注释掉**: 导致圆实体没有被正确创建
2. **dragged数组设置错误**: 固定点的参数没有正确加入dragged数组
3. **SLVS_C_WHERE_DRAGGED约束理解错误**: 这个约束不是用来固定点的

### SolveSpace的dragged数组机制
- `dragged`数组中的参数在求解时会被"拖拽"（即保持接近其当前值）
- 被拖拽的参数会尽量保持其位置不变
- 其他参数会移动以满足约束条件

## 修复方案

### 1. 恢复centerToCircleEntity映射

**文件**: `main/eageosolver.cpp`

```cpp
// 修复前：被注释掉
// std::map<int, int> centerToCircleEntity; // 圆心点ID到圆实体ID的映射

// 修复后：恢复映射
std::map<int, int> centerToCircleEntity; // 圆心点ID到圆实体ID的映射
```

### 2. 修复固定点约束处理

**文件**: `main/eageosolver.cpp`

```cpp
// 修复前：使用SLVS_C_WHERE_DRAGGED约束
else if (type == "fix_point") {
    int pointId = std::any_cast<int>(constraint.data.at("point"));
    // ... 添加SLVS_C_WHERE_DRAGGED约束
}

// 修复后：通过dragged数组实现
else if (type == "fix_point") {
    // 固定点通过dragged数组实现，不需要添加约束
    qDebug() << "GeometrySolver: Skipping fix_point constraint - will be handled by dragged array";
}
```

### 3. 修复dragged数组设置逻辑

**文件**: `main/eageosolver.cpp`

```cpp
// 修复前：只设置被拖拽点的参数
if (m_pointToParamX.find(draggedPointId) != m_pointToParamX.end() && m_pointToParamY.find(draggedPointId) != m_pointToParamY.end()) {
    m_sys.dragged[0] = m_pointToParamX[draggedPointId];
    m_sys.dragged[1] = m_pointToParamY[draggedPointId];
    m_sys.dragged[2] = 0;
    m_sys.dragged[3] = 0;
}

// 修复后：同时设置固定点的参数
if (m_pointToParamX.find(draggedPointId) != m_pointToParamX.end() && m_pointToParamY.find(draggedPointId) != m_pointToParamY.end()) {
    m_sys.dragged[0] = m_pointToParamX[draggedPointId];
    m_sys.dragged[1] = m_pointToParamY[draggedPointId];
    m_sys.dragged[2] = 0;
    m_sys.dragged[3] = 0;
}

// 将固定点（非拖拽点）的参数也加入dragged数组，使它们保持固定
int draggedIndex = 2;
for (auto it = m_pointToParamX.begin(); it != m_pointToParamX.end() && draggedIndex < 4; ++it) {
    int pointId = it->first;
    if (pointId != draggedPointId) {
        // 检查这个点是否有fix_point约束
        bool isFixed = false;
        for (const auto& constraint : constraints) {
            if (constraint.type == "fix_point") {
                int fixedPointId = std::any_cast<int>(constraint.data.at("point"));
                if (fixedPointId == pointId) {
                    isFixed = true;
                    break;
                }
            }
        }
        
        if (isFixed && draggedIndex < 4) {
            m_sys.dragged[draggedIndex++] = m_pointToParamX[pointId];
            if (draggedIndex < 4) {
                m_sys.dragged[draggedIndex++] = m_pointToParamY[pointId];
            }
            qDebug() << "GeometrySolver: Added fixed point" << pointId << "to dragged array";
        }
    }
}
```

## 修复后的行为

### 拖拽P2（圆上点）
- P2会沿圆周移动，保持与圆心距离30单位
- P1（圆心）的参数在dragged数组中，会保持固定
- 圆的位置和大小保持不变

### 拖拽P1（圆心点）
- P1的参数在dragged数组中，会保持固定
- 如果强制拖拽P1，P2会跟随移动以保持约束关系

## 技术细节

### dragged数组的作用
- `dragged[0]`, `dragged[1]`: 被拖拽点的X、Y参数
- `dragged[2]`, `dragged[3]`: 固定点的X、Y参数
- 求解器会尽量保持dragged数组中的参数接近其当前值

### 约束处理流程
1. 创建所有点和线段实体
2. 创建圆实体（为点在圆上约束）
3. 跳过`fix_point`约束（通过dragged数组实现）
4. 添加`pt_on_circle`约束（使用`SLVS_C_PT_ON_CIRCLE`）
5. 设置dragged数组：
   - 被拖拽点的参数
   - 固定点的参数
6. 调用求解器

## 调试信息

修复后系统会输出详细的调试信息：
```
GeometrySolver: Created circle with center point 1 radius 30 entity 303
GeometrySolver: Skipping fix_point constraint - will be handled by dragged array
GeometrySolver: Added point on circle constraint 2 for point 2 on circle with center 1 entities 301 303
GeometrySolver: Dragged point 2 parameters: 12 13
GeometrySolver: Added fixed point 1 to dragged array
GeometrySolver: Using dragged array to specify dragged parameters
```

## 相关文件修改

1. **`main/eageosolver.cpp`**: 
   - 恢复`centerToCircleEntity`映射
   - 修复固定点约束处理
   - 修复dragged数组设置逻辑

## 预期结果

现在圆约束功能应该能够正确工作：
1. P2会约束在圆周上移动
2. P1会保持固定不动
3. 约束求解会正确工作

这个修复确保了dragged数组能够正确设置，固定点会保持其位置，被拖拽的点会沿圆周移动。
