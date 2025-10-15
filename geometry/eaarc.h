#ifndef EAARC_H
#define EAARC_H

#include "eapoint.h"

class EaArc : public EaShape
{
public:
    EaArc();
    EaArc(EaPoint* center, double startAngle, double endAngle, double radius);

    bool onDrag(double x, double y) override;
    void onDraw(QPainter* painter) override;

    // 圆心和半径管理
    EaPoint* getCenter() const { return m_center; }
    double getRadius() const { return m_radius; }
    void setCenter(EaPoint* center);
    void setRadius(double radius);

    double getStartAngle() const { return m_startAngle; }
    double getEndAngle() const { return m_endAngle; }
    void setStartAngle(double angle);
    void setEndAngle(double angle);

    // ID管理
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }

    // 选择状态
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected) { m_selected = selected; }

private:
    EaPoint* m_center = nullptr;
    double m_startAngle = 0.0;
    double m_endAngle = 90.0;
    double m_radius = 1.0;
    int m_id = -1;
    bool m_selected = false;
};

#endif // EAARC_H
