#include "eadrawingarea.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QtMath>
#include <QDebug>

EaDrawingArea::EaDrawingArea(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setAntialiasing(true);  // 启用抗锯齿
    setRenderTarget(QQuickPaintedItem::FramebufferObject); // 性能优化
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
    GeometryPoint point;
    point.pos = QPointF(x, y);
    point.id = m_nextPointId++;
    m_points.append(point);
    update();
    
    qDebug() << "Added point" << point.id << "at" << point.pos;
}

void EaDrawingArea::addLine(int startId, int endId)
{
    GeometryLine line;
    line.startPointId = startId;
    line.endPointId = endId;
    m_lines.append(line);
    update();
    
    qDebug() << "Added line from point" << startId << "to" << endId;
}

void EaDrawingArea::clear()
{
    m_points.clear();
    m_lines.clear();
    m_nextPointId = 1;
    update();
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
    for (auto &point : m_points) {
        if (point.id == id) {
            point.pos = QPointF(x, y);
            update();
            return;
        }
    }
}

// ============ 绘制方法 ============

void EaDrawingArea::paint(QPainter *painter)
{
    // 设置渲染质量
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // 背景
    painter->fillRect(0, 0, width(), height(), Qt::white);
    
    // 绘制顺序：网格 -> 坐标轴 -> 线 -> 点
    if (m_showGrid) {
        drawGrid(painter);
    }
    
    drawCoordinateAxes(painter);
    drawLines(painter);
    drawPoints(painter);
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
    
    for (const auto &point : m_points) {
        QPointF screenPos = worldToScreen(point.pos.x(), point.pos.y());
        
        // 选择颜色
        QColor color = (point.selected || point.id == m_hoveredPointId) 
                       ? m_selectedPointColor : m_pointColor;
        
        // 如果是悬停点，绘制外圈
        if (point.id == m_hoveredPointId) {
            QPen hoverPen(m_selectedPointColor, 2.0);
            painter->setPen(hoverPen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(screenPos, 10, 10);
        }
        
        // 绘制点
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        double radius = point.selected ? 8 : 6;
        painter->drawEllipse(screenPos, radius, radius);
        
        // 绘制点ID标签
        painter->setPen(Qt::black);
        QFont font = painter->font();
        font.setPixelSize(10);
        painter->setFont(font);
        painter->drawText(screenPos + QPointF(12, 4), QString("P%1").arg(point.id));
    }
    
    painter->restore();
}

void EaDrawingArea::drawLines(QPainter *painter)
{
    painter->save();
    
    QPen linePen(m_lineColor, 2.0);
    painter->setPen(linePen);
    
    for (const auto &line : m_lines) {
        // 查找起点和终点
        QPointF startPos, endPos;
        bool foundStart = false, foundEnd = false;
        
        for (const auto &point : m_points) {
            if (point.id == line.startPointId) {
                startPos = worldToScreen(point.pos.x(), point.pos.y());
                foundStart = true;
            }
            if (point.id == line.endPointId) {
                endPos = worldToScreen(point.pos.x(), point.pos.y());
                foundEnd = true;
            }
            if (foundStart && foundEnd) break;
        }
        
        // 绘制线段
        if (foundStart && foundEnd) {
            if (line.selected) {
                linePen.setWidth(3);
                linePen.setColor(m_selectedPointColor);
                painter->setPen(linePen);
            }
            
            painter->drawLine(startPos, endPos);
            
            // 重置样式
            if (line.selected) {
                linePen.setWidth(2);
                linePen.setColor(m_lineColor);
                painter->setPen(linePen);
            }
        }
    }
    
    painter->restore();
}

// ============ 鼠标事件处理 ============

void EaDrawingArea::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
    
    if (event->button() == Qt::LeftButton) {
        // 检查是否点击了某个点
        QPointF worldPos = screenToWorld(event->pos().x(), event->pos().y());
        int pointId = findPointAt(event->pos());
        
        if (pointId >= 0) {
            // 开始拖拽点
            m_draggedPointId = pointId;
            for (auto &point : m_points) {
                if (point.id == pointId) {
                    point.isDragging = true;
                    point.selected = true;
                    emit pointClicked(pointId, point.pos.x(), point.pos.y());
                    break;
                }
            }
            update();
        }
    } else if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        // 中键或右键平移视图
        m_isPanning = true;
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
    
    QQuickPaintedItem::mousePressEvent(event);
}

void EaDrawingArea::mouseMoveEvent(QMouseEvent *event)
{
    QPointF delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();
    
    if (m_draggedPointId >= 0) {
        // 拖拽点
        QPointF worldPos = screenToWorld(event->pos().x(), event->pos().y());
        
        if (m_snapToGrid) {
            worldPos = snapToGridIfEnabled(worldPos);
        }
        
        for (auto &point : m_points) {
            if (point.id == m_draggedPointId) {
                point.pos = worldPos;
                emit pointDragged(m_draggedPointId, worldPos.x(), worldPos.y());
                break;
            }
        }
        update();
    } else if (m_isPanning) {
        // 平移视图
        m_panOffset += delta;
        update();
    }
    
    QQuickPaintedItem::mouseMoveEvent(event);
}

void EaDrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_draggedPointId >= 0) {
        // 结束拖拽
        for (auto &point : m_points) {
            if (point.id == m_draggedPointId) {
                point.isDragging = false;
                emit pointReleased(m_draggedPointId, point.pos.x(), point.pos.y());
                break;
            }
        }
        m_draggedPointId = -1;
        update();
    } else if ((event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) && m_isPanning) {
        // 结束平移
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
    for (const auto &point : m_points) {
        QPointF screenPos = worldToScreen(point.pos.x(), point.pos.y());
        double distance = QLineF(screenPos, pos).length();
        if (distance <= tolerance) {
            return point.id;
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
