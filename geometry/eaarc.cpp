#include "eaarc.h"

EaArc::EaArc() {}

EaArc::EaArc(EaPoint* center, double startAngle, double endAngle, double radius)
: m_center(center), m_radius(radius)
    , m_startAngle(startAngle)
    , m_endAngle(endAngle)
{

}

bool EaArc::onDrag(double x, double y)
{
    // 圆本身不直接拖拽，而是通过拖拽圆心来改变位置
    return false;
}

void EaArc::onDraw(QPainter* painter)
{
    if (!painter || !m_center) return;

    painter->save();

    // 选择颜色和线宽
    QColor color = m_selected ? QColor(244, 67, 54) : QColor(33, 150, 243); // 红色或蓝色
    double width = m_selected ? 3.0 : 2.0;

    QPen arcPen(color, width);
    painter->setPen(arcPen);

    // 获取圆心坐标
    QPointF centerPos(m_center->pos().x(), m_center->pos().y());

    // 计算圆弧的边界矩形
    QRectF arcRect(centerPos.x() - m_radius, centerPos.y() - m_radius, 
                   m_radius * 2, m_radius * 2);

    // 将角度从度转换为Qt的1/16度单位
    // Qt的drawArc使用1/16度作为单位，所以需要乘以16
    int startAngle16 = static_cast<int>(m_startAngle * 16);
    int spanAngle16 = static_cast<int>((m_endAngle - m_startAngle) * 16);

    // 绘制圆弧
    painter->drawArc(arcRect, startAngle16, spanAngle16);

    // 如果圆弧被选中，绘制圆心点
    if (m_selected) {
        painter->setPen(QPen(QColor(255, 0, 0), 4.0));
        painter->drawPoint(centerPos);
    }

    painter->restore();
}

void EaArc::setCenter(EaPoint* center)
{
    m_center = center;
}

void EaArc::setRadius(double radius)
{
    m_radius = radius;
}

void EaArc::setStartAngle(double angle)
{
    m_startAngle = angle;
}

void EaArc::setEndAngle(double angle)
{
    m_endAngle = angle;
}
