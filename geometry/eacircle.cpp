#include "eacircle.h"
#include <QPainter>
#include <QPen>
#include <QDebug>
#include <cmath>

EaCircle::EaCircle() 
{
}

EaCircle::EaCircle(EaPoint* center, double radius)
    : m_center(center), m_radius(radius)
{
}

bool EaCircle::onDrag(double x, double y)
{
    // 圆本身不直接拖拽，而是通过拖拽圆心来改变位置
    return false;
}

void EaCircle::onDraw(QPainter* painter)
{
    if (!painter || !m_center) return;
    
    painter->save();
    
    // 选择颜色和线宽
    QColor color = m_selected ? QColor(244, 67, 54) : QColor(33, 150, 243); // 红色或蓝色
    double width = m_selected ? 3.0 : 2.0;
    
    QPen circlePen(color, width);
    painter->setPen(circlePen);
    
    // 获取圆心坐标
    QPointF centerPos(m_center->pos().x(), m_center->pos().y());
    
    // 绘制圆
    // QPainter::drawEllipse需要的是矩形，所以需要计算左上角和宽高
    QRectF circleRect(centerPos.x() - m_radius, centerPos.y() - m_radius, 
                     2 * m_radius, 2 * m_radius);
    painter->drawEllipse(circleRect);
    
    // 如果圆被选中，绘制圆心点
    if (m_selected) {
        painter->setPen(QPen(QColor(255, 0, 0), 4.0));
        painter->drawPoint(centerPos);
    }
    
    painter->restore();
}

void EaCircle::setCenter(EaPoint* center)
{
    m_center = center;
}

void EaCircle::setRadius(double radius)
{
    m_radius = radius;
}