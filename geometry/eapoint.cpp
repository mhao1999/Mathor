#include "eapoint.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDebug>

EaPoint::EaPoint()
    : m_position(0.0, 0.0, 0.0)
{
}

EaPoint::EaPoint(double x, double y, double z)
    : m_position(x, y, z)
{
}

bool EaPoint::onDrag(double x, double y)
{
    setPosition(x, y, m_position.z());
    return true;
}

void EaPoint::onDraw(QPainter* painter)
{
    if (!painter) return;
    
    painter->save();
    
    // 选择颜色
    QColor color = m_selected ? QColor(244, 67, 54) : QColor(76, 175, 80); // 红色或绿色
    
    // 绘制点
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    double radius = m_selected ? 8 : 6;
    painter->drawEllipse(QPointF(m_position.x(), m_position.y()), radius, radius);
    
    // 绘制点ID标签
    if (m_id > 0) {
        painter->setPen(Qt::black);
        QFont font = painter->font();
        font.setPixelSize(10);
        painter->setFont(font);
        painter->drawText(QPointF(m_position.x() + 12, m_position.y() + 4), 
                         QString("P%1").arg(m_id));
    }
    
    painter->restore();
}

void EaPoint::setPosition(double x, double y, double z)
{
    m_position = Eigen::Vector3d(x, y, z);
}

void EaPoint::setPosition(const Eigen::Vector3d& position)
{
    m_position = position;
}
