#include "eadrawingarea.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QtMath>
#include <QDebug>

EaDrawingArea::EaDrawingArea(QQuickItem *parent)
    : QQuickPaintedItem(parent), m_session(EaSession::getInstance())
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setAntialiasing(true);  // 启用抗锯齿
    setRenderTarget(QQuickPaintedItem::FramebufferObject); // 性能优化
    
    // 重要：设置鼠标跟踪，这样即使没有按下鼠标按钮也能接收mouseMoveEvent
    // setMouseTracking(true);
    
    // 确保能够接收鼠标事件
    setFlag(QQuickItem::ItemAcceptsInputMethod, false);
    setFlag(QQuickItem::ItemIsFocusScope, false);
    
    // 连接EaSession的信号
    connect(m_session, &EaSession::geometryChanged, this, &EaDrawingArea::onGeometryChanged);
    
    qDebug() << "EaDrawingArea: Constructor completed, mouse tracking enabled";
}

// ============ 属性设置器 ============

void EaDrawingArea::setShowGrid(bool show)
{
    if (m_showGrid != show) {
        m_showGrid = show;
        emit showGridChanged();
        update();
    }
}

void EaDrawingArea::setGridSize(double size)
{
    if (!qFuzzyCompare(m_gridSize, size)) {
        m_gridSize = size;
        emit gridSizeChanged();
        update();
    }
}

void EaDrawingArea::setSnapToGrid(bool snap)
{
    if (m_snapToGrid != snap) {
        m_snapToGrid = snap;
        emit snapToGridChanged();
    }
}

void EaDrawingArea::setZoomLevel(double level)
{
    level = qBound(0.1, level, 10.0); // 限制缩放范围
    if (!qFuzzyCompare(m_zoomLevel, level)) {
        m_zoomLevel = level;
        emit zoomLevelChanged();
        update();
    }
}

// ============ QML 可调用方法 ============

void EaDrawingArea::addPoint(double x, double y)
{
    int pointId = m_session->addPoint(x, y, 0.0);
    qDebug() << "EaDrawingArea: Added point" << pointId << "at" << x << y;
}

void EaDrawingArea::addLine(int startId, int endId)
{
    int lineId = m_session->addLine(startId, endId);
    if (lineId >= 0) {
        qDebug() << "EaDrawingArea: Added line" << lineId << "from point" << startId << "to" << endId;
    }
}

void EaDrawingArea::clear()
{
    m_session->clear();
    qDebug() << "EaDrawingArea: Cleared all geometry";
}

QPointF EaDrawingArea::screenToWorld(double x, double y) const
{
    // 屏幕坐标转世界坐标
    double worldX = (x - width() / 2 - m_panOffset.x()) / m_zoomLevel;
    double worldY = -(y - height() / 2 - m_panOffset.y()) / m_zoomLevel; // Y轴翻转
    return QPointF(worldX, worldY);
}

QPointF EaDrawingArea::worldToScreen(double x, double y) const
{
    // 世界坐标转屏幕坐标
    double screenX = x * m_zoomLevel + width() / 2 + m_panOffset.x();
    double screenY = -y * m_zoomLevel + height() / 2 + m_panOffset.y(); // Y轴翻转
    return QPointF(screenX, screenY);
}

void EaDrawingArea::updatePointPosition(int id, double x, double y)
{
    m_session->updatePointPosition(id, x, y, 0.0);
}

// ============ 绘制方法 ============

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

void EaDrawingArea::drawShapes(QPainter *painter)
{
    painter->save();
    
    // 获取所有几何元素
    const auto& shapes = m_session->getShapes();
    
    // 统一绘制所有几何元素
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

void EaDrawingArea::drawPoint(QPainter *painter, std::shared_ptr<EaPoint> point)
{
    if (!painter || !point) return;
    
    QPointF screenPos = worldToScreen(point->pos().x(), point->pos().y());
    
    // 选择颜色
    QColor color = (point->isSelected() || point->getId() == m_hoveredPointId) 
                   ? m_selectedPointColor : m_pointColor;
    
    // 如果是悬停点，绘制外圈
    if (point->getId() == m_hoveredPointId) {
        QPen hoverPen(m_selectedPointColor, 2.0);
        painter->setPen(hoverPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(screenPos, 10, 10);
    }
    
    // 绘制点
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    double radius = point->isSelected() ? 8 : 6;
    painter->drawEllipse(screenPos, radius, radius);
    
    // 绘制点ID标签
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPixelSize(10);
    painter->setFont(font);
    painter->drawText(QPointF(screenPos.x() + 12, screenPos.y() + 4), 
                     QString("P%1").arg(point->getId()));
}

void EaDrawingArea::drawLine(QPainter *painter, std::shared_ptr<EaLine> line)
{
    if (!painter || !line) return;
    
    EaPoint* startPoint = line->getStartPoint();
    EaPoint* endPoint = line->getEndPoint();
    
    if (!startPoint || !endPoint) return;
    
    QPointF startPos = worldToScreen(startPoint->pos().x(), startPoint->pos().y());
    QPointF endPos = worldToScreen(endPoint->pos().x(), endPoint->pos().y());
    
    // 设置线条样式
    QPen linePen(m_lineColor, 2.0);
    if (line->isSelected()) {
        linePen.setWidth(3);
        linePen.setColor(m_selectedPointColor);
    }
    painter->setPen(linePen);
    
    // 绘制线段
    painter->drawLine(startPos, endPos);
}

void EaDrawingArea::drawCircle(QPainter *painter, std::shared_ptr<EaCircle> circle)
{
    if (!painter || !circle) return;
    
    EaPoint* centerPoint = circle->getCenter();
    if (!centerPoint) return;
    
    QPointF centerPos = worldToScreen(centerPoint->pos().x(), centerPoint->pos().y());
    double radius = circle->getRadius() * m_zoomLevel; // 根据缩放级别调整半径
    
    // 选择颜色和线宽
    QColor color = circle->isSelected() ? QColor(244, 67, 54) : QColor(33, 150, 243); // 红色或蓝色
    double width = circle->isSelected() ? 3.0 : 2.0;
    
    QPen circlePen(color, width);
    painter->setPen(circlePen);
    painter->setBrush(Qt::NoBrush); // 圆不填充
    
    // 绘制圆
    painter->drawEllipse(centerPos, radius, radius);
    
    // 如果圆被选中，绘制圆心点
    if (circle->isSelected()) {
        painter->setPen(QPen(QColor(255, 0, 0), 4.0));
        painter->drawPoint(centerPos);
    }
    
    // 绘制圆ID标签
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPixelSize(10);
    painter->setFont(font);
    painter->drawText(QPointF(centerPos.x() + radius + 5, centerPos.y() - 5), 
                     QString("C%1").arg(circle->getId()));
}

void EaDrawingArea::drawArc(QPainter *painter, std::shared_ptr<EaArc> arc)
{
    if (!painter || !arc) return;
    
    EaPoint* centerPoint = arc->getCenter();
    if (!centerPoint) return;
    
    QPointF centerPos = worldToScreen(centerPoint->pos().x(), centerPoint->pos().y());
    double radius = arc->getRadius() * m_zoomLevel; // 根据缩放级别调整半径
    
    // 选择颜色和线宽
    QColor color = arc->isSelected() ? QColor(244, 67, 54) : QColor(33, 150, 243); // 红色或蓝色
    double width = arc->isSelected() ? 3.0 : 2.0;
    
    QPen arcPen(color, width);
    painter->setPen(arcPen);
    
    // 计算圆弧的边界矩形
    QRectF arcRect(centerPos.x() - radius, centerPos.y() - radius, 
                   radius * 2, radius * 2);
    
    // 将角度从度转换为Qt的1/16度单位
    int startAngle16 = static_cast<int>(arc->getStartAngle() * 16);
    int spanAngle16 = static_cast<int>((arc->getEndAngle() - arc->getStartAngle()) * 16);
    
    // 绘制圆弧
    painter->drawArc(arcRect, startAngle16, spanAngle16);
    
    // 如果圆弧被选中，绘制圆心点
    if (arc->isSelected()) {
        painter->setPen(QPen(QColor(255, 0, 0), 4.0));
        painter->drawPoint(centerPos);
    }
    
    // 绘制圆弧ID标签
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPixelSize(10);
    painter->setFont(font);
    painter->drawText(QPointF(centerPos.x() + radius + 5, centerPos.y() - 5), 
                     QString("A%1").arg(arc->getId()));
}

void EaDrawingArea::drawGrid(QPainter *painter)
{
    painter->save();
    
    QPen gridPen(m_gridColor, 1.0);
    painter->setPen(gridPen);
    
    double spacing = m_gridSize * m_zoomLevel;
    
    // 绘制垂直网格线
    double startX = fmod(m_panOffset.x(), spacing);
    for (double x = startX; x < width(); x += spacing) {
        painter->drawLine(QPointF(x, 0), QPointF(x, height()));
    }
    
    // 绘制水平网格线
    double startY = fmod(m_panOffset.y(), spacing);
    for (double y = startY; y < height(); y += spacing) {
        painter->drawLine(QPointF(0, y), QPointF(width(), y));
    }
    
    painter->restore();
}

void EaDrawingArea::drawCoordinateAxes(QPainter *painter)
{
    painter->save();
    
    QPen axesPen(m_axesColor, 2.0);
    painter->setPen(axesPen);
    
    // 绘制X轴
    QPointF origin = worldToScreen(0, 0);
    painter->drawLine(QPointF(0, origin.y()), QPointF(width(), origin.y()));
    
    // 绘制Y轴
    painter->drawLine(QPointF(origin.x(), 0), QPointF(origin.x(), height()));
    
    // 绘制原点标记
    painter->setBrush(m_axesColor);
    painter->drawEllipse(origin, 4, 4);
    
    painter->restore();
}

void EaDrawingArea::drawPoints(QPainter *painter)
{
    painter->save();
    
    const auto& points = m_session->getPoints();
    for (const auto& point : points) {
        QPointF screenPos = worldToScreen(point->pos().x(), point->pos().y());
        
        // 选择颜色
        QColor color = (point->isSelected() || point->getId() == m_hoveredPointId) 
                       ? m_selectedPointColor : m_pointColor;
        
        // 如果是悬停点，绘制外圈
        if (point->getId() == m_hoveredPointId) {
            QPen hoverPen(m_selectedPointColor, 2.0);
            painter->setPen(hoverPen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(screenPos, 10, 10);
        }
        
        // 绘制点
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        double radius = point->isSelected() ? 8 : 6;
        painter->drawEllipse(screenPos, radius, radius);
        
        // 绘制点ID标签
        painter->setPen(Qt::black);
        QFont font = painter->font();
        font.setPixelSize(10);
        painter->setFont(font);
        painter->drawText(screenPos + QPointF(12, 4), QString("P%1").arg(point->getId()));
    }
    
    painter->restore();
}

void EaDrawingArea::drawLines(QPainter *painter)
{
    painter->save();
    
    const auto& lines = m_session->getLines();
    for (const auto& line : lines) {
        EaPoint* startPoint = line->getStartPoint();
        EaPoint* endPoint = line->getEndPoint();
        
        if (!startPoint || !endPoint) continue;
        
        QPointF startPos = worldToScreen(startPoint->pos().x(), startPoint->pos().y());
        QPointF endPos = worldToScreen(endPoint->pos().x(), endPoint->pos().y());
        
        // 设置线条样式
        QPen linePen(m_lineColor, 2.0);
        if (line->isSelected()) {
            linePen.setWidth(3);
            linePen.setColor(m_selectedPointColor);
        }
        painter->setPen(linePen);
        
        // 绘制线段
        painter->drawLine(startPos, endPos);
    }
    
    painter->restore();
}

void EaDrawingArea::drawCircles(QPainter *painter)
{
    painter->save();
    
    const auto& circles = m_session->getCircles();
    for (const auto& circle : circles) {
        EaPoint* centerPoint = circle->getCenter();
        if (!centerPoint) continue;
        
        QPointF centerPos = worldToScreen(centerPoint->pos().x(), centerPoint->pos().y());
        double radius = circle->getRadius() * m_zoomLevel; // 根据缩放级别调整半径
        
        // 选择颜色和线宽
        QColor color = circle->isSelected() ? QColor(244, 67, 54) : QColor(33, 150, 243); // 红色或蓝色
        double width = circle->isSelected() ? 3.0 : 2.0;
        
        QPen circlePen(color, width);
        painter->setPen(circlePen);
        painter->setBrush(Qt::NoBrush); // 圆不填充
        
        // 绘制圆
        painter->drawEllipse(centerPos, radius, radius);
        
        // 如果圆被选中，绘制圆心点
        if (circle->isSelected()) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(244, 67, 54)); // 红色填充
            painter->drawEllipse(centerPos, 4, 4);
        }
        
        // 绘制圆ID标签
        painter->setPen(Qt::black);
        QFont font = painter->font();
        font.setPixelSize(10);
        painter->setFont(font);
        painter->drawText(centerPos + QPointF(radius + 5, -5), QString("C%1").arg(circle->getId()));
    }
    
    painter->restore();
}

// ============ 鼠标事件处理 ============

void EaDrawingArea::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "EaDrawingArea: mousePressEvent at" << event->pos() << "button:" << event->button();
    m_lastMousePos = event->pos();
    
    if (event->button() == Qt::LeftButton) {
        // 检查是否点击了某个点
        QPointF worldPos = screenToWorld(event->pos().x(), event->pos().y());
        int pointId = findPointAt(event->pos());
        
        qDebug() << "EaDrawingArea: Left button pressed, worldPos:" << worldPos << "pointId:" << pointId;
        
        if (pointId >= 0) {
            // 开始拖拽点
            m_draggedPointId = pointId;
            qDebug() << "EaDrawingArea: Starting drag for point" << pointId;
            EaPoint* point = m_session->getPoint(pointId);
            if (point) {
                point->setDragging(true);
                point->setSelected(true);
                m_session->selectPoint(pointId, true);
                emit pointClicked(pointId, point->pos().x(), point->pos().y());
            }
            update();
        }
    } else if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        // 中键或右键平移视图
        m_isPanning = true;
        setCursor(QCursor(Qt::ClosedHandCursor));
        qDebug() << "EaDrawingArea: Starting panning";
    }
    
    // QQuickPaintedItem::mousePressEvent(event);
}

void EaDrawingArea::mouseMoveEvent(QMouseEvent *event)
{
    // 简单的鼠标移动测试 - 每10次移动输出一次，避免日志过多
    static int moveCount = 0;
    if (++moveCount % 10 == 0) {
        qDebug() << "EaDrawingArea: mouseMoveEvent at" << event->pos() << "draggedPointId:" << m_draggedPointId << "isPanning:" << m_isPanning;
    }
    
    QPointF delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();
    
    if (m_draggedPointId >= 0) {
        // 拖拽点
        QPointF worldPos = screenToWorld(event->pos().x(), event->pos().y());
        
        if (m_snapToGrid) {
            worldPos = snapToGridIfEnabled(worldPos);
        }
        
        qDebug() << "EaDrawingArea: Dragging point" << m_draggedPointId << "to worldPos:" << worldPos;
        
        EaPoint* point = m_session->getPoint(m_draggedPointId);
        if (point) {
            // 使用约束感知的拖拽方法
            bool success = point->onDragWithConstraints(worldPos.x(), worldPos.y());
            if (success) {
                emit pointDragged(m_draggedPointId, worldPos.x(), worldPos.y());
            }
        }
        update();
    } else if (m_isPanning) {
        // 平移视图
        m_panOffset += delta;
        qDebug() << "EaDrawingArea: Panning, new offset:" << m_panOffset;
        update();
    }
    
    QQuickPaintedItem::mouseMoveEvent(event);
}

void EaDrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "EaDrawingArea: mouseReleaseEvent button:" << event->button() << "draggedPointId:" << m_draggedPointId;
    
    if (event->button() == Qt::LeftButton && m_draggedPointId >= 0) {
        // 结束拖拽
        qDebug() << "EaDrawingArea: Ending drag for point" << m_draggedPointId;
        EaPoint* point = m_session->getPoint(m_draggedPointId);
        if (point) {
            point->setDragging(false);
            emit pointReleased(m_draggedPointId, point->pos().x(), point->pos().y());
        }
        m_draggedPointId = -1;
        update();
    } else if ((event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) && m_isPanning) {
        // 结束平移
        qDebug() << "EaDrawingArea: Ending panning";
        m_isPanning = false;
        setCursor(QCursor(Qt::ArrowCursor));
    }
    
    QQuickPaintedItem::mouseReleaseEvent(event);
}

void EaDrawingArea::wheelEvent(QWheelEvent *event)
{
    // 鼠标滚轮缩放
    double zoomFactor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    setZoomLevel(m_zoomLevel * zoomFactor);
    
    QQuickPaintedItem::wheelEvent(event);
}

void EaDrawingArea::hoverMoveEvent(QHoverEvent *event)
{
    // 更新悬停状态
    int oldHoveredId = m_hoveredPointId;
    m_hoveredPointId = findPointAt(event->pos());
    
    if (m_hoveredPointId >= 0) {
        setCursor(QCursor(Qt::PointingHandCursor));
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
    }
    
    if (oldHoveredId != m_hoveredPointId) {
        update();
    }
    
    QQuickPaintedItem::hoverMoveEvent(event);
}

// ============ 辅助方法 ============

int EaDrawingArea::findPointAt(const QPointF &pos, double tolerance)
{
    const auto& points = m_session->getPoints();
    for (const auto& point : points) {
        QPointF screenPos = worldToScreen(point->pos().x(), point->pos().y());
        double distance = QLineF(screenPos, pos).length();
        if (distance <= tolerance) {
            return point->getId();
        }
    }
    return -1;
}

QPointF EaDrawingArea::snapToGridIfEnabled(const QPointF &pos)
{
    if (!m_snapToGrid) {
        return pos;
    }
    
    double snappedX = qRound(pos.x() / m_gridSize) * m_gridSize;
    double snappedY = qRound(pos.y() / m_gridSize) * m_gridSize;
    return QPointF(snappedX, snappedY);
}

void EaDrawingArea::updateTransform()
{
    // 预留用于更复杂的变换
    update();
}

void EaDrawingArea::onGeometryChanged()
{
    update();
}
