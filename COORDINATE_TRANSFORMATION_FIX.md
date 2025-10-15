# 坐标转换修复说明

## 问题描述

用户反馈：点击约束按钮后，画出来的几何元素都是屏幕坐标了，不是之前正确的世界坐标了，需要转换一下。

## 问题分析

### 根本原因
在之前的优化中，我们将`EaDrawingArea::paint`方法简化为使用统一的`drawShapes`方法，但是这个方法直接调用了各个几何元素的`onDraw`方法：

```cpp
// 问题代码
void EaDrawingArea::drawShapes(QPainter *painter)
{
    for (const auto& shape : shapes) {
        if (shape) {
            shape->onDraw(painter);  // 直接调用，没有坐标转换
        }
    }
}
```

### 坐标系统问题
- **几何元素内部**: 使用世界坐标（如`m_position.x()`, `m_position.y()`）
- **Qt绘制系统**: 需要屏幕坐标
- **原有分类绘制方法**: 有坐标转换（`worldToScreen`）
- **新的统一绘制方法**: 缺少坐标转换

## 解决方案

### 1. 修改统一绘制方法

#### **使用类型检测和专用绘制方法**
```cpp
void EaDrawingArea::drawShapes(QPainter *painter)
{
    painter->save();
    
    const auto& shapes = m_session->getShapes();
    
    for (const auto& shape : shapes) {
        if (shape) {
            // 根据几何元素类型进行坐标转换
            if (auto point = std::dynamic_pointer_cast<EaPoint>(shape)) {
                drawPoint(painter, point);
            } else if (auto line = std::dynamic_pointer_cast<EaLine>(shape)) {
                drawLine(painter, line);
            } else if (auto circle = std::dynamic_pointer_cast<EaCircle>(shape)) {
                drawCircle(painter, circle);
            } else if (auto arc = std::dynamic_pointer_cast<EaArc>(shape)) {
                drawArc(painter, arc);
            }
        }
    }
    
    painter->restore();
}
```

### 2. 实现专用绘制方法

#### **drawPoint方法**
```cpp
void EaDrawingArea::drawPoint(QPainter *painter, std::shared_ptr<EaPoint> point)
{
    if (!painter || !point) return;
    
    // 关键：世界坐标转屏幕坐标
    QPointF screenPos = worldToScreen(point->pos().x(), point->pos().y());
    
    // 选择颜色
    QColor color = (point->isSelected() || point->getId() == m_hoveredPointId) 
                   ? m_selectedPointColor : m_pointColor;
    
    // 绘制点（使用屏幕坐标）
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    double radius = point->isSelected() ? 8 : 6;
    painter->drawEllipse(screenPos, radius, radius);
    
    // 绘制点ID标签（使用屏幕坐标）
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPixelSize(10);
    painter->setFont(font);
    painter->drawText(QPointF(screenPos.x() + 12, screenPos.y() + 4), 
                     QString("P%1").arg(point->getId()));
}
```

#### **drawLine方法**
```cpp
void EaDrawingArea::drawLine(QPainter *painter, std::shared_ptr<EaLine> line)
{
    if (!painter || !line) return;
    
    EaPoint* startPoint = line->getStartPoint();
    EaPoint* endPoint = line->getEndPoint();
    
    if (!startPoint || !endPoint) return;
    
    // 关键：世界坐标转屏幕坐标
    QPointF startPos = worldToScreen(startPoint->pos().x(), startPoint->pos().y());
    QPointF endPos = worldToScreen(endPoint->pos().x(), endPoint->pos().y());
    
    // 设置线条样式
    QPen linePen(m_lineColor, 2.0);
    if (line->isSelected()) {
        linePen.setWidth(3);
        linePen.setColor(m_selectedPointColor);
    }
    painter->setPen(linePen);
    
    // 绘制线段（使用屏幕坐标）
    painter->drawLine(startPos, endPos);
}
```

#### **drawCircle方法**
```cpp
void EaDrawingArea::drawCircle(QPainter *painter, std::shared_ptr<EaCircle> circle)
{
    if (!painter || !circle) return;
    
    EaPoint* centerPoint = circle->getCenter();
    if (!centerPoint) return;
    
    // 关键：世界坐标转屏幕坐标
    QPointF centerPos = worldToScreen(centerPoint->pos().x(), centerPoint->pos().y());
    double radius = circle->getRadius() * m_zoomLevel; // 根据缩放级别调整半径
    
    // 选择颜色和线宽
    QColor color = circle->isSelected() ? QColor(244, 67, 54) : QColor(33, 150, 243);
    double width = circle->isSelected() ? 3.0 : 2.0;
    
    QPen circlePen(color, width);
    painter->setPen(circlePen);
    painter->setBrush(Qt::NoBrush);
    
    // 绘制圆（使用屏幕坐标）
    painter->drawEllipse(centerPos, radius, radius);
    
    // 绘制圆心点和ID标签
    if (circle->isSelected()) {
        painter->setPen(QPen(QColor(255, 0, 0), 4.0));
        painter->drawPoint(centerPos);
    }
    
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPixelSize(10);
    painter->setFont(font);
    painter->drawText(QPointF(centerPos.x() + radius + 5, centerPos.y() - 5), 
                     QString("C%1").arg(circle->getId()));
}
```

#### **drawArc方法**
```cpp
void EaDrawingArea::drawArc(QPainter *painter, std::shared_ptr<EaArc> arc)
{
    if (!painter || !arc) return;
    
    EaPoint* centerPoint = arc->getCenter();
    if (!centerPoint) return;
    
    // 关键：世界坐标转屏幕坐标
    QPointF centerPos = worldToScreen(centerPoint->pos().x(), centerPoint->pos().y());
    double radius = arc->getRadius() * m_zoomLevel; // 根据缩放级别调整半径
    
    // 选择颜色和线宽
    QColor color = arc->isSelected() ? QColor(244, 67, 54) : QColor(33, 150, 243);
    double width = arc->isSelected() ? 3.0 : 2.0;
    
    QPen arcPen(color, width);
    painter->setPen(arcPen);
    
    // 计算圆弧的边界矩形（使用屏幕坐标）
    QRectF arcRect(centerPos.x() - radius, centerPos.y() - radius, 
                   radius * 2, radius * 2);
    
    // 将角度从度转换为Qt的1/16度单位
    int startAngle16 = static_cast<int>(arc->getStartAngle() * 16);
    int spanAngle16 = static_cast<int>((arc->getEndAngle() - arc->getStartAngle()) * 16);
    
    // 绘制圆弧（使用屏幕坐标）
    painter->drawArc(arcRect, startAngle16, spanAngle16);
    
    // 绘制圆心点和ID标签
    if (arc->isSelected()) {
        painter->setPen(QPen(QColor(255, 0, 0), 4.0));
        painter->drawPoint(centerPos);
    }
    
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPixelSize(10);
    painter->setFont(font);
    painter->drawText(QPointF(centerPos.x() + radius + 5, centerPos.y() - 5), 
                     QString("A%1").arg(arc->getId()));
}
```

### 3. 完善EaSession支持

#### **添加圆弧容器**
```cpp
// 保持原有的分类存储以兼容现有代码
std::vector<std::shared_ptr<EaPoint>> m_points;
std::vector<std::shared_ptr<EaLine>> m_lines;
std::vector<std::shared_ptr<EaCircle>> m_circles;
std::vector<std::shared_ptr<EaArc>> m_arcs;  // 新增
```

#### **添加圆弧访问方法**
```cpp
const std::vector<std::shared_ptr<EaArc>>& getArcs() const { return m_arcs; }
```

#### **完善addArc方法**
```cpp
int EaSession::addArc(int centerPointId, double radius, double start, double end)
{
    // 验证圆心是否存在
    EaPoint* centerPoint = getPoint(centerPointId);
    if (!centerPoint) {
        qWarning() << "EaSession: Cannot create arc - invalid center point ID:" << centerPointId;
        return -1;
    }

    auto arc = std::make_shared<EaArc>();
    arc->setCenter(centerPoint);
    arc->setRadius(radius);
    arc->setStartAngle(start);
    arc->setEndAngle(end);
    arc->setId(m_nextCircleId);

    // 添加到分类容器
    m_arcs.push_back(arc);
    // 添加到统一容器
    m_shapes.push_back(std::static_pointer_cast<EaShape>(arc));

    int arcId = m_nextCircleId++;
    emit arcAdded(arcId, centerPointId, radius, start, end);
    emit geometryChanged();

    qDebug() << "EaSession: Added arc" << arcId << "with center point" << centerPointId << "radius" << radius;
    return arcId;
}
```

## 关键修复点

### 1. 坐标转换
- **问题**: 直接调用`shape->onDraw(painter)`，没有坐标转换
- **解决**: 使用`worldToScreen()`进行坐标转换

### 2. 类型安全
- **问题**: 使用`dynamic_pointer_cast`进行类型检测
- **解决**: 为每种几何类型提供专用的绘制方法

### 3. 缩放处理
- **问题**: 半径需要根据缩放级别调整
- **解决**: 使用`radius * m_zoomLevel`

### 4. 角度转换
- **问题**: 圆弧角度需要转换为Qt的1/16度单位
- **解决**: 使用`angle * 16`进行转换

## 优化效果

### 1. 坐标正确性
- ✅ 所有几何元素现在使用正确的屏幕坐标
- ✅ 支持缩放和平移变换
- ✅ 保持与世界坐标系统的一致性

### 2. 代码结构
- ✅ 保持了统一绘制的优势
- ✅ 每种几何类型有专用的绘制方法
- ✅ 易于维护和扩展

### 3. 性能
- ✅ 使用`dynamic_pointer_cast`进行类型检测
- ✅ 避免了重复的坐标转换
- ✅ 保持了原有的绘制性能

## 测试验证

### 验证步骤
1. 点击各种约束按钮
2. 检查几何元素是否在正确位置显示
3. 测试缩放和平移功能
4. 验证圆弧是否正确绘制

### 预期结果
- 所有几何元素在正确的位置显示
- 缩放和平移功能正常工作
- 圆弧按指定角度正确绘制
- 坐标系统保持一致

## 总结

这次修复解决了统一绘制方法中缺少坐标转换的问题，通过为每种几何类型提供专用的绘制方法，既保持了代码的统一性，又确保了坐标转换的正确性。这是一个很好的例子，说明了在优化代码结构时需要仔细考虑所有相关的技术细节。
