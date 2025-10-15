#ifndef EACIRCLE_H
#define EACIRCLE_H

#include "eashape.h"
#include "eapoint.h"

class EaCircle : public EaShape
{
public:
    EaCircle();
    EaCircle(EaPoint* center, double radius);

    bool onDrag(double x, double y) override;
    void onDraw(QPainter* painter) override;

    // 圆心和半径管理
    EaPoint* getCenter() const { return m_center; }
    double getRadius() const { return m_radius; }
    void setCenter(EaPoint* center);
    void setRadius(double radius);
    
    // ID管理
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    
    // 选择状态
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected) { m_selected = selected; }

private:
    EaPoint* m_center = nullptr;
    double m_radius = 0.0;
    int m_id = -1;
    bool m_selected = false;
};

#endif // EACIRCLE_H