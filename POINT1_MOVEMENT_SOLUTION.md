# 点1移动问题解决方案

## 问题描述
点1比之前更容易移动了，跟着点2满场跑。

## 问题分析

### 根本原因
我们只有1个距离约束，但系统有4个自由度：
- 点1的X、Y坐标（2个自由度）
- 点2的X、Y坐标（2个自由度）
- 距离约束（1个约束）

**结果**：系统有4-1=3个自由度，所以点1和点2都可以自由移动来满足距离约束。

### 解决方案
需要添加更多约束来固定点1，使系统只有1个自由度（点2沿圆弧移动）。

## 约束系统设计

### 方案1：使用SLVS_C_WHERE_DRAGGED约束（已实施）
```cpp
// 添加点1的固定约束
m_sys.constraint[m_sys.constraints++] = Slvs_MakeConstraint(
    constraintId, g,
    SLVS_C_WHERE_DRAGGED,
    200,
    0.0,
    pointToEntity[1], 0, 0, 0);
```

**约束系统**：
1. 距离约束：点1和点2之间距离为100
2. 点1固定约束：使用`SLVS_C_WHERE_DRAGGED`固定点1
3. 拖拽约束：点2的参数在`dragged`数组中

**系统自由度**：
- 总自由度：4
- 距离约束：1个
- 点1固定约束：1个
- 拖拽约束：点2参数被标记为拖拽参数
- 预期DOF：4-2=2

### 方案2：使用SLVS_C_PT_IN_PLANE约束
```cpp
// 添加点1的固定约束
m_sys.constraint[m_sys.constraints++] = Slvs_MakeConstraint(
    constraintId, g,
    SLVS_C_PT_IN_PLANE,
    200,
    0.0,
    pointToEntity[1], 0, 0, 0);
```

### 方案3：使用dragged数组固定点1
```cpp
// 固定点1的参数
m_sys.dragged[0] = pointToParamX[1];  // 固定点1的X参数
m_sys.dragged[1] = pointToParamY[1];  // 固定点1的Y参数
m_sys.dragged[2] = pointToParamX[2];  // 拖拽点2的X参数
m_sys.dragged[3] = pointToParamY[2];  // 拖拽点2的Y参数
```

## 测试验证

### 预期调试输出
```
GeometrySolver: Added point1 fixed constraint 2 for entity 300 at position x1 y1
GeometrySolver: Total constraints added: 2
GeometrySolver: Dragged point 2 parameters: 12 13
```

### 预期结果
- DOF应该为2或更少
- 点1应该保持固定
- 点2可以沿圆弧移动
- 两点间距离始终为100

## 调试步骤

### 步骤1：检查约束数量
确保总约束数量为2（1个距离约束 + 1个点1固定约束）

### 步骤2：检查DOF
DOF应该小于4，理想情况下为1或2

### 步骤3：验证点1固定
点1的坐标应该保持不变

### 步骤4：验证点2移动
点2应该沿着以点1为圆心的圆弧移动

## 如果问题仍然存在

### 可能的原因
1. 约束类型不正确
2. 约束参数设置错误
3. SolveSpaceLib版本问题
4. 约束冲突

### 调试方法
1. 尝试不同的约束类型
2. 检查约束参数
3. 查看SolveSpaceLib示例
4. 添加更多调试信息

## 最终目标
- 点1完全固定不动
- 点2沿着以点1为圆心的圆弧移动
- 两点间距离始终为100
- DOF = 1（只有点2可以移动）
