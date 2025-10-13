#include "ealine.h"
#include <QPainter>
#include <QPen>
#include <QDebug>

EaLine::EaLine() 
{
}

EaLine::EaLine(EaPoint* startPoint, EaPoint* endPoint)
    : m_startPoint(startPoint), m_endPoint(endPoint)
{
}

bool EaLine::onDrag(double x, double y)
{
    // 线段本身不直接拖拽，而是通过拖拽端点来改变
    return false;
}

void EaLine::onDraw(QPainter* painter)
{
    if (!painter || !m_startPoint || !m_endPoint) return;
    
    painter->save();
    
    // 选择颜色和线宽
    QColor color = m_selected ? QColor(244, 67, 54) : QColor(33, 150, 243); // 红色或蓝色
    double width = m_selected ? 3.0 : 2.0;
    
    QPen linePen(color, width);
    painter->setPen(linePen);
    
    // 绘制线段
    QPointF startPos(m_startPoint->pos().x(), m_startPoint->pos().y());
    QPointF endPos(m_endPoint->pos().x(), m_endPoint->pos().y());
    painter->drawLine(startPos, endPos);
    
    painter->restore();
}

void EaLine::setStartPoint(EaPoint* point)
{
    m_startPoint = point;
}

void EaLine::setEndPoint(EaPoint* point)
{
    m_endPoint = point;
}

int EaLine::getStartPointId() const
{
    return m_startPoint ? m_startPoint->getId() : -1;
}

int EaLine::getEndPointId() const
{
    return m_endPoint ? m_endPoint->getId() : -1;
}
