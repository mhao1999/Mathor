# 简化约束方法说明

## 用户反馈

用户反馈`dragged`数组从来没有起过作用，建议不要使用它。

## 新的方法

### 完全移除dragged数组的使用

**文件**: `main/eageosolver.cpp`

```cpp
// 修复前：复杂的dragged数组设置
// 使用dragged数组来指定被拖拽的参数
if (m_pointToParamX.find(draggedPointId) != m_pointToParamX.end() && m_pointToParamY.find(draggedPointId) != m_pointToParamY.end()) {
    m_sys.dragged[0] = m_pointToParamX[draggedPointId];
    m_sys.dragged[1] = m_pointToParamY[draggedPointId];
    // ... 复杂的固定点处理
}

// 修复后：简单的约束求解
// 不使用dragged数组，直接进行约束求解
// 清空dragged数组
for (int i = 0; i < 4; i++) {
    m_sys.dragged[i] = 0;
}

qDebug() << "GeometrySolver: Not using dragged array - relying on constraints only";
```

### 使用SLVS_C_WHERE_DRAGGED约束固定点

**文件**: `main/eageosolver.cpp`

```cpp
// 恢复fix_point约束处理
else if (type == "fix_point") {
    int pointId = std::any_cast<int>(constraint.data.at("point"));
    
    if (pointToEntity.find(pointId) != pointToEntity.end()) {
        // 添加固定点约束 - 使用WHERE_DRAGGED约束来固定点
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

## 约束系统说明

### 约束类型
1. **fix_point**: 使用`SLVS_C_WHERE_DRAGGED`约束固定点
2. **pt_on_circle**: 使用`SLVS_C_PT_ON_CIRCLE`约束点在圆上
3. **parallel**: 使用`SLVS_C_PARALLEL`约束线段平行

### 求解机制
- 不使用`dragged`数组
- 完全依靠约束来定义几何关系
- 求解器会根据约束自动调整点的位置

## 预期行为

### 拖拽P2（圆上点）
- P2会沿圆周移动，保持与圆心距离30单位
- P1（圆心）有`SLVS_C_WHERE_DRAGGED`约束，会保持固定
- 圆的位置和大小保持不变

### 拖拽P1（圆心点）
- P1有`SLVS_C_WHERE_DRAGGED`约束，会保持固定
- 如果强制拖拽P1，P2会跟随移动以保持约束关系

## 调试信息

系统会输出详细的调试信息：
```
GeometrySolver: Added fix point constraint 1 for point 1 entity 300
GeometrySolver: Added point on circle constraint 2 for point 2 on circle with center 1 entities 301 303
GeometrySolver: Total constraints added: 2
GeometrySolver: Not using dragged array - relying on constraints only
```

## 技术优势

### 简化性
- 移除了复杂的`dragged`数组逻辑
- 约束系统更加直观和简单
- 减少了出错的可能性

### 可靠性
- 完全依靠SolveSpace的约束系统
- 不依赖可能不工作的`dragged`数组
- 约束关系更加明确

### 可维护性
- 代码更简洁
- 逻辑更清晰
- 更容易调试和修改

## 相关文件修改

1. **`main/eageosolver.cpp`**: 
   - 恢复`centerToCircleEntity`映射
   - 恢复`fix_point`约束处理
   - 移除复杂的`dragged`数组逻辑
   - 简化为纯约束求解

## 测试验证

现在圆约束功能应该能够正确工作：
1. P2会约束在圆周上移动
2. P1会保持固定不动
3. 约束求解会正确工作
4. 不依赖可能不工作的`dragged`数组

这个简化的方法更加可靠和直观，完全依靠SolveSpace的约束系统来实现几何关系。
