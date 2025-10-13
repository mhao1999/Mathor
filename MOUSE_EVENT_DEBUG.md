# 鼠标事件调试指南

## 问题描述
mouseMoveEvent方法无法被调用，导致拖拽功能无法正常工作。

## 可能的原因和解决方案

### 1. 鼠标跟踪未启用
**问题**：QQuickPaintedItem默认不启用鼠标跟踪
**解决方案**：在构造函数中添加 `setMouseTracking(true)`

```cpp
EaDrawingArea::EaDrawingArea(QQuickItem *parent)
    : QQuickPaintedItem(parent), m_session(EaSession::getInstance())
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setMouseTracking(true);  // 重要：启用鼠标跟踪
    // ... 其他设置
}
```

### 2. 鼠标事件被其他组件拦截
**问题**：QML中的其他组件可能拦截了鼠标事件
**解决方案**：检查QML层次结构，确保DrawingArea在最上层

```qml
Rectangle {
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#f5f5f5"
    
    DrawingArea {
        id: drawingArea
        anchors.fill: parent
        anchors.margins: 10
        // 确保没有其他组件覆盖
    }
}
```

### 3. 事件过滤器问题
**问题**：可能有事件过滤器阻止了鼠标事件
**解决方案**：检查是否有其他组件安装了事件过滤器

### 4. QQuickPaintedItem的特殊要求
**问题**：QQuickPaintedItem可能需要特殊的设置
**解决方案**：添加必要的标志设置

```cpp
// 确保能够接收鼠标事件
setFlag(QQuickItem::ItemAcceptsInputMethod, false);
setFlag(QQuickItem::ItemIsFocusScope, false);
```

## 调试步骤

### 步骤1：验证构造函数
检查控制台是否输出：
```
EaDrawingArea: Constructor completed, mouse tracking enabled
```

### 步骤2：测试鼠标按下事件
移动鼠标到DrawingArea上并点击，检查是否输出：
```
EaDrawingArea: mousePressEvent at QPoint(x, y) button: 1
```

### 步骤3：测试鼠标移动事件
在DrawingArea上移动鼠标，检查是否输出：
```
EaDrawingArea: mouseMoveEvent at QPoint(x, y) draggedPointId: -1 isPanning: false
```

### 步骤4：测试拖拽功能
1. 添加一个点
2. 点击该点
3. 拖拽该点
4. 检查是否输出拖拽相关的调试信息

## 常见问题排查

### 问题1：完全没有鼠标事件
**可能原因**：
- DrawingArea被其他组件覆盖
- 鼠标事件被父组件拦截
- QML层次结构问题

**解决方案**：
```qml
// 确保DrawingArea在最上层
Rectangle {
    // 父容器
    DrawingArea {
        z: 1  // 设置较高的z-order
        // ...
    }
}
```

### 问题2：只有mousePressEvent，没有mouseMoveEvent
**可能原因**：
- 没有启用鼠标跟踪
- 鼠标移动速度太快
- 事件处理被中断

**解决方案**：
```cpp
// 确保启用鼠标跟踪
setMouseTracking(true);

// 添加更详细的调试信息
void EaDrawingArea::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << "Mouse move event received at" << event->pos();
    // ... 处理逻辑
}
```

### 问题3：拖拽状态不正确
**可能原因**：
- m_draggedPointId没有正确设置
- 点选择逻辑有问题

**解决方案**：
```cpp
void EaDrawingArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int pointId = findPointAt(event->pos());
        if (pointId >= 0) {
            m_draggedPointId = pointId;  // 确保正确设置
            qDebug() << "Starting drag for point" << pointId;
        }
    }
}
```

## 测试代码

### 简单的鼠标事件测试
```cpp
void EaDrawingArea::mouseMoveEvent(QMouseEvent *event)
{
    static int count = 0;
    if (++count % 50 == 0) {  // 每50次移动输出一次
        qDebug() << "Mouse moved" << count << "times to" << event->pos();
    }
    QQuickPaintedItem::mouseMoveEvent(event);
}
```

### 拖拽测试
```cpp
void EaDrawingArea::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Mouse pressed at" << event->pos();
    if (event->button() == Qt::LeftButton) {
        int pointId = findPointAt(event->pos());
        if (pointId >= 0) {
            m_draggedPointId = pointId;
            qDebug() << "Will drag point" << pointId;
        }
    }
    QQuickPaintedItem::mousePressEvent(event);
}
```

## 预期输出

### 正常情况下的控制台输出
```
EaDrawingArea: Constructor completed, mouse tracking enabled
EaDrawingArea: mousePressEvent at QPoint(100, 100) button: 1
EaDrawingArea: Left button pressed, worldPos: QPointF(10, 20) pointId: 1
EaDrawingArea: Starting drag for point 1
EaDrawingArea: mouseMoveEvent at QPoint(105, 105) draggedPointId: 1 isPanning: false
EaDrawingArea: Dragging point 1 to worldPos: QPointF(10.5, 20.5)
EaSession: solveDragConstraint called for point 1 to position 10.5 20.5
EaDrawingArea: mouseReleaseEvent button: 1 draggedPointId: 1
EaDrawingArea: Ending drag for point 1
```

如果看到这些输出，说明鼠标事件处理正常工作。
