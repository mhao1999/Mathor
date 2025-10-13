# 拖拽约束修复说明

## 问题分析

从调试输出可以看出：
```
GeometrySolver: solveDragConstraint called for point 2 to position 151 94
GeometrySolver: Created point 1 at 9.92341 19.9234 with params 7 8
GeometrySolver: Using new position for dragged point 2 : 151 94
GeometrySolver: Created point 2 at 151 94 with params 9 10
GeometrySolver: Setting drag constraints for point 2 params: 9 10
GeometrySolver: drag constraint solve successfully!
GeometrySolver: pt 1 after solve: ( 9.92341 ,  19.9234 )
GeometrySolver: pt 2 after solve: ( 151 ,  94 ), pt 2 coordinates always change as mouse cursor.
```

**问题**：点2的坐标总是跟随鼠标光标移动，约束没有起作用。

## 根本原因

**SolveSpaceLib的拖拽约束机制理解错误**：

### 错误的实现（之前）：
```cpp
// 错误：将被拖拽的点设置为"dragged"
m_sys.dragged[0] = draggedParamX;  // 点2的X参数
m_sys.dragged[1] = draggedParamY;  // 点2的Y参数
```

### 正确的理解：
在SolveSpaceLib中，`dragged`数组包含的是**应该被固定的参数**，而不是被拖拽的参数。

- `dragged`数组中的参数会被固定，不能移动
- **不在**`dragged`数组中的参数可以自由移动
- 被拖拽的点应该**不**在`dragged`数组中

## 修复方案

### 修复后的实现：
```cpp
// 正确：固定不被拖拽的点
int draggedCount = 0;
for (auto it = pointPositions.begin(); it != pointPositions.end(); ++it) {
    int pointId = it.key().toInt();
    if (pointId != draggedPointId) {
        // 这个点不是被拖拽的点，应该被固定
        if (pointToParamX.contains(pointId) && pointToParamY.contains(pointId)) {
            if (draggedCount < 4) {
                m_sys.dragged[draggedCount++] = pointToParamX[pointId];
                if (draggedCount < 4) {
                    m_sys.dragged[draggedCount++] = pointToParamY[pointId];
                }
            }
        }
    }
}
```

### 工作原理：

1. **拖拽点2时**：
   - 点1的参数被添加到`dragged`数组中（被固定）
   - 点2的参数**不**在`dragged`数组中（可以自由移动）
   - SolveSpaceLib让点2移动到新位置
   - 点1保持固定，但可能根据约束微调位置

2. **约束求解**：
   - 点2移动到鼠标位置
   - 距离约束确保两点间距离为100
   - 如果点2的新位置违反约束，SolveSpaceLib会调整点2到约束允许的位置

## 预期结果

修复后，拖拽点2时应该看到：

### 调试输出：
```
GeometrySolver: Setting drag constraints - fixed 2 parameters
GeometrySolver: Dragged point 2 is free to move
GeometrySolver: drag constraint solve successfully!
GeometrySolver: pt 1 after solve: ( 10 ,  20 )  // 点1基本固定
GeometrySolver: pt 2 after solve: ( 约束调整后的位置 )  // 点2在约束允许的圆上
```

### 视觉效果：
- 点2在拖拽时沿着以点1为圆心、半径为100的圆弧移动
- 点1基本保持固定（可能根据约束微调）
- 线段长度始终保持100

## 测试步骤

1. **编译并运行程序**
2. **创建测试场景**：
   - 添加点1和点2
   - 添加距离约束
3. **测试拖拽**：
   - 拖拽点2
   - 观察点2是否沿着圆弧移动
4. **检查调试输出**：
   - 确认"fixed X parameters"（X > 0）
   - 确认"Dragged point 2 is free to move"

## 技术细节

### SolveSpaceLib拖拽机制：
- `dragged`数组最多支持4个参数
- 每个点需要2个参数（X和Y坐标）
- 最多可以固定2个点（4个参数）

### 约束求解流程：
1. 设置被拖拽点的新位置
2. 固定其他点的参数
3. 应用几何约束
4. 求解系统，调整被拖拽点到约束允许的位置

这个修复应该解决点2总是跟随鼠标移动的问题，让它正确地沿着约束圆弧移动。
