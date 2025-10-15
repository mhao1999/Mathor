#ifndef EADRAWINGAREA_H
#define EADRAWINGAREA_H

#include <QQuickPaintedItem>
#include <QPainter>
#include <QPointF>
#include <QMouseEvent>
#include "easession.h"

/**
 * @brief 几何绘制区域 - 支持交互式几何元素绘制和编辑
 */
class EaDrawingArea : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
    Q_PROPERTY(double gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(bool snapToGrid READ snapToGrid WRITE setSnapToGrid NOTIFY snapToGridChanged)
    Q_PROPERTY(double zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)

public:
    // 几何元素类型
    enum ElementType {
        Point,
        Line,
        Circle,
        Arc
    };
    Q_ENUM(ElementType)

    explicit EaDrawingArea(QQuickItem *parent = nullptr);
    
    // 属性访问器
    bool showGrid() const { return m_showGrid; }
    void setShowGrid(bool show);
    
    double gridSize() const { return m_gridSize; }
    void setGridSize(double size);
    
    bool snapToGrid() const { return m_snapToGrid; }
    void setSnapToGrid(bool snap);
    
    double zoomLevel() const { return m_zoomLevel; }
    void setZoomLevel(double level);

    // QML 可调用方法
    Q_INVOKABLE void addPoint(double x, double y);
    Q_INVOKABLE void addLine(int startId, int endId);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QPointF screenToWorld(double x, double y) const;
    Q_INVOKABLE QPointF worldToScreen(double x, double y) const;
    Q_INVOKABLE void updatePointPosition(int id, double x, double y);

    // 绘制接口
    void paint(QPainter *painter) override;

signals:
    void showGridChanged();
    void gridSizeChanged();
    void snapToGridChanged();
    void zoomLevelChanged();
    
    void pointClicked(int pointId, double x, double y);
    void pointDragged(int pointId, double x, double y);
    void pointReleased(int pointId, double x, double y);
    void elementSelected(int elementId, int elementType);

protected:
    // 鼠标事件处理
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;

private slots:
    void onGeometryChanged();

private:
    // 绘制辅助方法
    void drawGrid(QPainter *painter);
    // 统一绘制方法
    void drawShapes(QPainter *painter);
    
    // 单个几何元素绘制方法（带坐标转换）
    void drawPoint(QPainter *painter, std::shared_ptr<EaPoint> point);
    void drawLine(QPainter *painter, std::shared_ptr<EaLine> line);
    void drawCircle(QPainter *painter, std::shared_ptr<EaCircle> circle);
    void drawArc(QPainter *painter, std::shared_ptr<EaArc> arc);
    
    // 保持原有的分类绘制方法以兼容现有代码
    void drawPoints(QPainter *painter);
    void drawLines(QPainter *painter);
    void drawCircles(QPainter *painter);
    void drawCoordinateAxes(QPainter *painter);
    
    // 交互辅助方法
    int findPointAt(const QPointF &pos, double tolerance = 10.0);
    QPointF snapToGridIfEnabled(const QPointF &pos);
    void updateTransform();

    // EaSession引用
    EaSession* m_session;
    
    // 视图属性
    bool m_showGrid = true;
    double m_gridSize = 20.0;
    bool m_snapToGrid = false;
    double m_zoomLevel = 1.0;
    QPointF m_panOffset = QPointF(0, 0);
    
    // 交互状态
    int m_draggedPointId = -1;
    int m_hoveredPointId = -1;
    QPointF m_lastMousePos;
    bool m_isPanning = false;
    
    // 视觉样式
    QColor m_gridColor = QColor(230, 230, 230);
    QColor m_axesColor = QColor(150, 150, 150);
    QColor m_pointColor = QColor(76, 175, 80);      // 绿色
    QColor m_selectedPointColor = QColor(244, 67, 54); // 红色
    QColor m_lineColor = QColor(33, 150, 243);      // 蓝色
};

#endif // EADRAWINGAREA_H
