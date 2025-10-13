# 点1移动问题分析

## 问题描述
在拖拽点2的过程中，点1不应该移动，但实际上点1也在移动。

## 可能的原因

### 1. 坐标分配错误
**问题**：在约束求解结果处理中，我们按照参数在数组中的顺序来分配坐标，而不是按照点ID。

**原始错误代码**：
```cpp
// 按照参数数组顺序分配坐标
if (pointCount == 0) {
    m_solvedX1 = m_sys.param[i].val;  // 可能是点2的X坐标
} else if (pointCount == 1) {
    m_solvedY1 = m_sys.param[i].val;  // 可能是点2的Y坐标
}
```

**修复后的代码**：
```cpp
// 按照点ID分配坐标
for (auto it = pointToParamX.begin(); it != pointToParamX.end(); ++it) {
    int pointId = it.key();
    if (it.value() == m_sys.param[i].h) {
        if (pointId == 1) {
            m_solvedX1 = m_sys.param[i].val;  // 确保是点1的X坐标
        } else if (pointId == 2) {
            m_solvedX2 = m_sys.param[i].val;  // 确保是点2的X坐标
        }
    }
}
```

### 2. 约束不足
**问题**：只有1个距离约束，但系统有4个自由度（2个点的X、Y坐标）。

**分析**：
- 系统总自由度：4（点1的X、Y，点2的X、Y）
- 距离约束：1个
- 拖拽约束：2个（固定点1的X、Y）
- 剩余自由度：4 - 1 - 2 = 1

**结果**：点1可能仍然有1个自由度可以移动。

### 3. SolveSpaceLib求解器行为
**问题**：SolveSpaceLib可能选择移动点1来满足约束，而不是保持点1固定。

## 解决方案

### 方案1：修复坐标分配（已实施）
确保求解结果按照正确的点ID分配坐标。

### 方案2：添加更多约束
如果点1应该完全固定，可以添加：
- 点1的X坐标约束
- 点1的Y坐标约束

### 方案3：检查拖拽约束设置
确保点1的参数被正确添加到`dragged`数组中。

## 测试验证

### 测试1：检查拖拽约束设置
运行程序后，观察调试输出：
```
GeometrySolver: Setting drag constraints - fixed 2 parameters
GeometrySolver: Fixed parameter 0: 10  // 点1的X参数
GeometrySolver: Fixed parameter 1: 11  // 点1的Y参数
GeometrySolver: Point 1 X param: 10 Y param: 11
GeometrySolver: Point 2 X param: 12 Y param: 13
```

### 测试2：检查坐标分配
观察求解后的坐标：
```
GeometrySolver: pt 1 after solve: ( 128.498 ,  54.2873 )
GeometrySolver: pt 2 after solve: ( 228.29 ,  60.7288 )
```

如果点1的坐标没有变化，说明拖拽约束正确。

### 测试3：验证距离约束
计算两点间距离：
```
distance = sqrt((228.29 - 128.498)^2 + (60.7288 - 54.2873)^2)
distance = sqrt(99.792^2 + 6.5015^2) = sqrt(9960.8 + 42.27) = sqrt(10003.07) ≈ 100.015
```

距离应该接近100.0。

## 预期结果

修复后，应该看到：
1. 点1的坐标保持不变
2. 点2沿着以点1为圆心的圆弧移动
3. 两点间距离始终为100.0
4. DOF = 1（只有点2可以沿圆弧移动）

## 调试输出分析

如果问题仍然存在，观察以下调试信息：
- 拖拽约束是否正确设置
- 坐标分配是否正确
- 约束求解是否成功
- 自由度是否正确
