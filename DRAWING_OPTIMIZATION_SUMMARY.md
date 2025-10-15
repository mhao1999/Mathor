# 绘制代码优化总结

## 用户问题

用户询问`EaDrawingArea::paint`能否优化，并指出既然所有几何类都从`EaShape`中派生，是否有必要在`EaSession`中分类型存储几何元素。

## 优化方案

### 核心思想
既然所有几何类（`EaPoint`、`EaLine`、`EaCircle`、`EaArc`等）都从`EaShape`派生，我们可以：
1. 使用统一的容器存储所有几何元素
2. 简化绘制代码，使用统一的绘制循环
3. 保持向后兼容性

## 实现内容

### 1. EaSession统一存储

#### **添加统一容器**
```cpp
// 几何元素存储 - 统一容器
std::vector<std::shared_ptr<EaShape>> m_shapes;

// 保持原有的分类存储以兼容现有代码
std::vector<std::shared_ptr<EaPoint>> m_points;
std::vector<std::shared_ptr<EaLine>> m_lines;
std::vector<std::shared_ptr<EaCircle>> m_circles;
```

#### **添加统一访问方法**
```cpp
// 统一几何元素访问
const std::vector<std::shared_ptr<EaShape>>& getShapes() const { return m_shapes; }

// 保持原有的分类访问方法以兼容现有代码
const std::vector<std::shared_ptr<EaPoint>>& getPoints() const { return m_points; }
const std::vector<std::shared_ptr<EaLine>>& getLines() const { return m_lines; }
const std::vector<std::shared_ptr<EaCircle>>& getCircles() const { return m_circles; }
```

### 2. 更新添加方法

#### **addPoint方法**
```cpp
int EaSession::addPoint(double x, double y, double z)
{
    auto point = std::make_shared<EaPoint>();
    point->setPosition(x, y, z);
    point->setId(m_nextPointId);
    
    // 添加到分类容器
    m_points.push_back(point);
    // 添加到统一容器
    m_shapes.push_back(std::static_pointer_cast<EaShape>(point));
    
    int pointId = m_nextPointId++;
    // ... 其他代码
    return pointId;
}
```

#### **addLine方法**
```cpp
int EaSession::addLine(int startPointId, int endPointId)
{
    // ... 验证代码
    
    auto line = std::make_shared<EaLine>();
    line->setStartPoint(startPoint);
    line->setEndPoint(endPoint);
    line->setId(m_nextLineId);
    
    // 添加到分类容器
    m_lines.push_back(line);
    // 添加到统一容器
    m_shapes.push_back(std::static_pointer_cast<EaShape>(line));
    
    int lineId = m_nextLineId++;
    // ... 其他代码
    return lineId;
}
```

#### **addCircle方法**
```cpp
int EaSession::addCircle(int centerPointId, double radius)
{
    // ... 验证代码
    
    auto circle = std::make_shared<EaCircle>();
    circle->setCenter(centerPoint);
    circle->setRadius(radius);
    circle->setId(m_nextCircleId);
    
    // 添加到分类容器
    m_circles.push_back(circle);
    // 添加到统一容器
    m_shapes.push_back(std::static_pointer_cast<EaShape>(circle));
    
    int circleId = m_nextCircleId++;
    // ... 其他代码
    return circleId;
}
```

### 3. 简化绘制代码

#### **优化前的paint方法**
```cpp
void EaDrawingArea::paint(QPainter *painter)
{
    // 设置渲染质量
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // 背景
    painter->fillRect(0, 0, width(), height(), Qt::white);
    
    // 绘制顺序：网格 -> 坐标轴 -> 线 -> 圆 -> 点
    if (m_showGrid) {
        drawGrid(painter);
    }
    
    drawCoordinateAxes(painter);
    drawLines(painter);      // 单独绘制线段
    drawCircles(painter);    // 单独绘制圆
    drawPoints(painter);     // 单独绘制点
}
```

#### **优化后的paint方法**
```cpp
void EaDrawingArea::paint(QPainter *painter)
{
    // 设置渲染质量
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // 背景
    painter->fillRect(0, 0, width(), height(), Qt::white);
    
    // 绘制顺序：网格 -> 坐标轴 -> 几何元素
    if (m_showGrid) {
        drawGrid(painter);
    }
    
    drawCoordinateAxes(painter);
    
    // 统一绘制所有几何元素
    drawShapes(painter);
}
```

#### **新增drawShapes方法**
```cpp
void EaDrawingArea::drawShapes(QPainter *painter)
{
    painter->save();
    
    // 获取所有几何元素
    const auto& shapes = m_session->getShapes();
    
    // 统一绘制所有几何元素
    for (const auto& shape : shapes) {
        if (shape) {
            shape->onDraw(painter);
        }
    }
    
    painter->restore();
}
```

## 优化效果

### 1. 代码简化

#### **绘制代码减少**
- **优化前**: 需要分别调用`drawPoints`、`drawLines`、`drawCircles`
- **优化后**: 只需要调用`drawShapes`

#### **维护性提升**
- 新增几何类型时，不需要修改绘制代码
- 所有几何元素使用统一的绘制流程

### 2. 性能优化

#### **减少函数调用**
- **优化前**: 3个独立的绘制函数调用
- **优化后**: 1个统一的绘制函数调用

#### **减少重复代码**
- 每个几何类型都有自己的绘制循环
- 现在使用统一的绘制循环

### 3. 扩展性提升

#### **新增几何类型**
- 只需要实现`EaShape::onDraw`方法
- 自动支持统一绘制

#### **绘制顺序控制**
- 可以通过调整`m_shapes`容器的顺序来控制绘制顺序
- 或者通过Z-order属性来控制

## 向后兼容性

### 保持原有接口
- 所有原有的分类访问方法仍然可用
- 所有原有的分类绘制方法仍然可用
- 现有代码不需要修改

### 渐进式迁移
- 可以逐步将代码迁移到使用统一接口
- 新功能优先使用统一接口

## 未来扩展

### 1. 绘制顺序控制
```cpp
// 可以按类型分组绘制
void EaDrawingArea::drawShapes(QPainter *painter)
{
    painter->save();
    
    const auto& shapes = m_session->getShapes();
    
    // 按绘制顺序分组
    std::vector<std::shared_ptr<EaShape>> lines, circles, points;
    
    for (const auto& shape : shapes) {
        if (auto line = std::dynamic_pointer_cast<EaLine>(shape)) {
            lines.push_back(shape);
        } else if (auto circle = std::dynamic_pointer_cast<EaCircle>(shape)) {
            circles.push_back(shape);
        } else if (auto point = std::dynamic_pointer_cast<EaPoint>(shape)) {
            points.push_back(shape);
        }
    }
    
    // 按顺序绘制：线 -> 圆 -> 点
    for (const auto& shape : lines) shape->onDraw(painter);
    for (const auto& shape : circles) shape->onDraw(painter);
    for (const auto& shape : points) shape->onDraw(painter);
    
    painter->restore();
}
```

### 2. 性能优化
```cpp
// 可以添加可见性检测
void EaDrawingArea::drawShapes(QPainter *painter)
{
    painter->save();
    
    const auto& shapes = m_session->getShapes();
    QRectF viewport = getViewport();
    
    for (const auto& shape : shapes) {
        if (shape && isShapeVisible(shape, viewport)) {
            shape->onDraw(painter);
        }
    }
    
    painter->restore();
}
```

### 3. 分层绘制
```cpp
// 可以支持分层绘制
void EaDrawingArea::drawShapes(QPainter *painter)
{
    painter->save();
    
    const auto& shapes = m_session->getShapes();
    
    // 按层分组绘制
    for (int layer = 0; layer < maxLayers; ++layer) {
        for (const auto& shape : shapes) {
            if (shape && shape->getLayer() == layer) {
                shape->onDraw(painter);
            }
        }
    }
    
    painter->restore();
}
```

## 总结

这次优化实现了：

1. **代码简化**: 绘制代码从3个函数减少到1个函数
2. **性能提升**: 减少函数调用开销
3. **扩展性**: 新增几何类型自动支持
4. **兼容性**: 保持所有原有接口
5. **维护性**: 统一的绘制流程，易于维护

这是一个很好的重构示例，展示了如何利用面向对象的多态性来简化代码结构。
