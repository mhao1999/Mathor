# 平行约束崩溃问题修复

## 问题描述

当拖拽点4时，程序崩溃并显示错误：
```
File D:\qt\SolveSpaceLib\libslvs\dsc.h, line 334, function FindById:
Assertion 'Cannot find handle' failed: ((t != NULL) == false).
```

## 问题原因分析

### 1. 约束冲突
原始实现中，我们为所有4个点都添加了`SLVS_C_WHERE_DRAGGED`约束：
```cpp
this->createDragConstraint(pt1);
this->createDragConstraint(pt2);
this->createDragConstraint(pt3);
this->createDragConstraint(pt4);
```

这导致了约束冲突，因为`SLVS_C_WHERE_DRAGGED`约束应该只用于被拖拽的点。

### 2. 约束ID冲突
在`GeometrySolver`中，约束ID的生成使用了`m_sys.constraints + 1`，这可能导致ID冲突。

### 3. 重复约束
为同一个实体添加了多个相同类型的约束，导致SolveSpace库无法正确处理。

## 修复方案

### 1. 移除多余的拖拽约束
```cpp
// 修复前：为所有点添加拖拽约束
this->createDragConstraint(pt1);
this->createDragConstraint(pt2);
this->createDragConstraint(pt3);
this->createDragConstraint(pt4);

// 修复后：不添加拖拽约束，通过dragged数组处理
// 注意：我们不在这里为所有点添加拖拽约束
// 拖拽约束只在solveDragConstraint中动态处理
```

### 2. 修复约束ID生成
```cpp
// 修复前：使用m_sys.constraints + 1
int constraintId = m_sys.constraints + 1;

// 修复后：使用独立的计数器
int constraintIdCounter = 1;
int constraintId = constraintIdCounter++;
```

### 3. 简化约束处理逻辑
```cpp
// 修复前：为drag_point类型添加SLVS_C_WHERE_DRAGGED约束
else if (type == "drag_point") {
    // 添加可拖拽点约束 - 使用拖拽约束
    m_sys.constraint[m_sys.constraints++] = Slvs_MakeConstraint(...);
}

// 修复后：跳过drag_point约束，通过dragged数组处理
else if (type == "drag_point") {
    // 拖拽约束现在通过dragged数组处理，不需要添加SLVS_C_WHERE_DRAGGED约束
    qDebug() << "GeometrySolver: Skipping drag_point constraint - handled by dragged array";
}
```

## 修复后的约束系统

### 约束类型
1. **平行约束** (`SLVS_C_PARALLEL`): 确保两条线段保持平行
2. **拖拽处理**: 通过`dragged`数组指定被拖拽的参数

### 系统自由度
- 总自由度：8（4个点的X、Y坐标）
- 平行约束：1个
- 拖拽约束：2个（被拖拽点的X、Y参数）
- 剩余自由度：8 - 1 - 2 = 5

### 预期行为
- 被拖拽的点移动到新位置
- 其他点根据平行约束自动调整
- 两条线段始终保持平行

## 测试验证

### 测试步骤
1. 点击"平行约束"按钮创建场景
2. 点击"测试拖拽约束"按钮测试功能
3. 观察控制台输出，确认没有约束冲突

### 预期调试输出
```
EaSession: Created parallel constraint with points 1 2 3 4
EaSession: Created lines 1 and 2 with parallel constraint
GeometrySolver: Processing constraint type: parallel constraint id: 1
GeometrySolver: Added parallel constraint 1 between lines 1 and 2 entities 304 305
GeometrySolver: Total constraints added: 1
GeometrySolver: Dragged point 4 parameters: 16 17
```

## 关键改进

1. **简化约束系统**: 只添加必要的约束，避免冲突
2. **正确的拖拽处理**: 使用`dragged`数组而不是`SLVS_C_WHERE_DRAGGED`约束
3. **独立的ID管理**: 避免约束ID冲突
4. **清晰的约束逻辑**: 每种约束类型都有明确的处理方式

这个修复确保了平行约束功能能够正常工作，不会因为约束冲突而导致程序崩溃。
