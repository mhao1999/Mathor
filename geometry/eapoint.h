#ifndef EAPOINT_H
#define EAPOINT_H

#include "eashape.h"
#include <Eigen/Dense>

class EaPoint : public EaShape
{
public:
    EaPoint();
    EaPoint(double x, double y, double z = 0.0);

    bool onDrag(double x, double y) override;
    void onDraw(QPainter* painter) override;

    // 位置访问器
    const Eigen::Vector3d& pos() const { return m_position; }
    void setPosition(double x, double y, double z = 0.0);
    void setPosition(const Eigen::Vector3d& position);
    
    // ID管理
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    
    // 选择状态
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected) { m_selected = selected; }
    
    // 拖拽状态
    bool isDragging() const { return m_isDragging; }
    void setDragging(bool dragging) { m_isDragging = dragging; }

private:
    Eigen::Vector3d m_position;
    int m_id = -1;
    bool m_selected = false;
    bool m_isDragging = false;
};

#endif // EAPOINT_H
