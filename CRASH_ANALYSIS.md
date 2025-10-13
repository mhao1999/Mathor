# 崩溃问题分析

## 问题描述
程序在约束求解时崩溃，错误信息：
```
File D:\qt\SolveSpaceLib\libslvs\dsc.h, line 334, function FindById:
Assertion 'Cannot find handle' failed: ((t != NULL) == false).
```

## 问题原因

### 1. 重复约束
从调试输出可以看到：
```
GeometrySolver: Added point1 fixed constraint 2 for entity 300
GeometrySolver: Added point1 fixed constraint 3 for entity 300
```

我们为同一个实体（点1）添加了两次固定约束，这导致了约束冲突。

### 2. 约束类型错误
`SLVS_C_PT_IN_PLANE`约束可能不是固定点的正确方法，或者参数设置不正确。

## 解决方案

### 方案1：使用dragged数组（已实施）
回到使用`dragged`数组的方法，这是SolveSpaceLib推荐的方式：

```cpp
// 固定点1的参数
m_sys.dragged[0] = pointToParamX[1];  // 固定点1的X参数
m_sys.dragged[1] = pointToParamY[1];  // 固定点1的Y参数
m_sys.dragged[2] = 0;  // 未使用
m_sys.dragged[3] = 0;  // 未使用
```

### 方案2：正确的约束类型
如果必须使用约束，应该使用：
- `SLVS_C_WHERE_DRAGGED`：专门用于拖拽操作
- `SLVS_C_PT_PT_DISTANCE`：距离约束
- 避免使用`SLVS_C_PT_IN_PLANE`，因为它可能不是固定点的正确方法

## 约束系统分析

### 当前约束：
1. **距离约束**：点1和点2之间距离为100
2. **拖拽约束**：通过`dragged`数组固定点1的参数

### 系统自由度：
- 总自由度：4（点1的X、Y，点2的X、Y）
- 距离约束：1个
- 拖拽约束：2个（固定点1的X、Y参数）
- 剩余自由度：4 - 1 - 2 = 1

### 预期行为：
- 点1完全固定（X、Y参数在`dragged`数组中）
- 点2可以沿圆弧移动（1个自由度）

## 测试验证

### 预期调试输出：
```
GeometrySolver: Fixed point1 parameters: 10 11
GeometrySolver: Using dragged array to fix point1
GeometrySolver: Dragged point 2 is free to move
GeometrySolver: Point 1 X param: 10 Y param: 11
GeometrySolver: Point 2 X param: 12 Y param: 13
```

### 预期结果：
- 程序不再崩溃
- 点1保持固定
- 点2沿圆弧移动
- DOF = 1

## 调试技巧

### 1. 约束数量检查
确保约束数量正确，避免重复约束。

### 2. 参数ID验证
确保所有参数ID都有效，没有引用不存在的参数。

### 3. 实体ID验证
确保所有实体ID都有效，没有引用不存在的实体。

### 4. 约束类型验证
使用正确的约束类型，避免使用未定义或不正确的约束类型。

## 下一步

如果问题仍然存在，可能需要：
1. 检查SolveSpaceLib的文档
2. 查看示例代码中的约束使用方法
3. 尝试更简单的约束组合
4. 添加更多的调试信息来定位问题
