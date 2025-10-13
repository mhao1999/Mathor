# 拖拽约束问题修复

## 问题描述
当拖拽点2时，无论鼠标光标位置在哪里，几何约束得到的新坐标还是和第一次求解得到的坐标一样，点2并没有沿着圆弧移动。

## 问题原因分析

### 1. 鼠标事件处理问题
**问题**：用户注释掉了关键的鼠标事件处理代码
- `setMouseTracking(true)` 被注释掉
- `QQuickPaintedItem::mousePressEvent(event)` 被注释掉
- 鼠标事件处理方法声明被移除

**影响**：导致鼠标事件无法正常接收和处理

### 2. 约束求解参数映射问题
**问题**：在GeometrySolver的`solveDragConstraint`方法中，拖拽约束的参数映射逻辑有误
- 没有正确记录每个点对应的参数索引
- 拖拽约束设置时使用了错误的参数索引
- 导致被拖拽的点没有被正确固定

**影响**：约束求解器无法识别哪个点应该被拖拽，导致求解结果不正确

## 修复方案

### 1. 恢复鼠标事件处理
```cpp
// 在构造函数中恢复
setMouseTracking(true);

// 在mousePressEvent中恢复
QQuickPaintedItem::mousePressEvent(event);

// 在头文件中恢复声明
void mousePressEvent(QMouseEvent *event) override;
void mouseMoveEvent(QMouseEvent *event) override;
void mouseReleaseEvent(QMouseEvent *event) override;
```

### 2. 修复约束求解参数映射
```cpp
// 创建参数映射表
QMap<int, int> pointToParamX; // 点ID到X参数索引的映射
QMap<int, int> pointToParamY; // 点ID到Y参数索引的映射

// 创建参数时记录索引
int paramXIndex = m_sys.params;
m_sys.param[m_sys.params++] = Slvs_MakeParam(paramIndex++, g, x);
int paramYIndex = m_sys.params;
m_sys.param[m_sys.params++] = Slvs_MakeParam(paramIndex++, g, y);

// 记录参数索引
pointToParamX[pointId] = paramXIndex;
pointToParamY[pointId] = paramYIndex;

// 设置拖拽约束时使用正确的参数索引
if (pointToParamX.contains(draggedPointId) && pointToParamY.contains(draggedPointId)) {
    int draggedParamX = pointToParamX[draggedPointId];
    int draggedParamY = pointToParamY[draggedPointId];
    
    m_sys.dragged[0] = draggedParamX;
    m_sys.dragged[1] = draggedParamY;
    m_sys.dragged[2] = 0;
    m_sys.dragged[3] = 0;
}
```

### 3. 添加详细的调试信息
```cpp
qDebug() << "GeometrySolver: solveDragConstraint called for point" << draggedPointId 
         << "to position" << newX << newY;
qDebug() << "GeometrySolver: Using new position for dragged point" << pointId << ":" << x << y;
qDebug() << "GeometrySolver: Setting drag constraints for point" << draggedPointId 
         << "params:" << draggedParamX << draggedParamY;
```

## 修复后的工作流程

### 1. 鼠标事件处理
1. 用户点击点2 → `mousePressEvent` 被调用
2. 设置 `m_draggedPointId = 2`
3. 用户拖拽鼠标 → `mouseMoveEvent` 被调用
4. 调用 `point->onDragWithConstraints(worldPos.x(), worldPos.y(), m_session)`

### 2. 约束求解
1. `EaSession::solveDragConstraint` 被调用
2. 构建点位置映射和约束列表
3. `GeometrySolver::solveDragConstraint` 被调用
4. 创建SolveSpaceLib系统，正确设置拖拽约束
5. 求解约束，点2在距离点1为100.0的圆上移动
6. 更新点的位置

### 3. 预期结果
- 点2在拖拽时沿着以点1为圆心、半径为100.0的圆弧移动
- 线段长度始终保持100.0
- 控制台显示详细的调试信息

## 测试步骤

### 1. 编译并运行程序
```bash
cd D:\qt\Mathor
qmake
nmake
# 运行生成的Mathor.exe
```

### 2. 创建测试场景
1. 点击"添加点1"按钮
2. 点击"添加点2"按钮
3. 点击"添加线段"按钮（会自动添加距离约束）

### 3. 测试拖拽约束
1. 点击点2开始拖拽
2. 移动鼠标，观察点2是否沿着圆弧移动
3. 检查控制台输出

### 4. 预期控制台输出
```
EaDrawingArea: mousePressEvent at QPoint(x, y) button: 1
EaDrawingArea: Starting drag for point 2
EaDrawingArea: mouseMoveEvent at QPoint(x, y) draggedPointId: 2 isPanning: false
EaSession: solveDragConstraint called for point 2 to position [x] [y]
GeometrySolver: solveDragConstraint called for point 2 to position [x] [y]
GeometrySolver: Using new position for dragged point 2: [x] [y]
GeometrySolver: Setting drag constraints for point 2 params: [paramX] [paramY]
GeometrySolver: 拖拽约束求解成功!
GeometrySolver: 求解后点1: ([x1], [y1])
GeometrySolver: 求解后点2: ([x2], [y2])
```

## 注意事项

1. **鼠标事件必须启用**：`setMouseTracking(true)` 是必需的
2. **参数映射必须正确**：每个点的参数索引必须正确记录和使用
3. **调试信息很重要**：通过控制台输出可以确认每个步骤是否正常执行
4. **约束系统一致性**：确保约束系统不会过约束或产生矛盾

修复后，拖拽约束功能应该能够正常工作，点2会在约束允许的圆弧上移动。
