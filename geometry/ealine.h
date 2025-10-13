#ifndef EALINE_H
#define EALINE_H

#include "eashape.h"
#include "eapoint.h"

class EaLine : public EaShape
{
public:
    EaLine();
    EaLine(EaPoint* startPoint, EaPoint* endPoint);

    bool onDrag(double x, double y) override;
    void onDraw(QPainter* painter) override;

    // 起点终点管理
    EaPoint* getStartPoint() const { return m_startPoint; }
    EaPoint* getEndPoint() const { return m_endPoint; }
    void setStartPoint(EaPoint* point);
    void setEndPoint(EaPoint* point);
    
    // ID管理
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    
    // 获取起点终点ID
    int getStartPointId() const;
    int getEndPointId() const;
    
    // 选择状态
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected) { m_selected = selected; }

private:
    EaPoint* m_startPoint = nullptr;
    EaPoint* m_endPoint = nullptr;
    int m_id = -1;
    bool m_selected = false;
};

#endif // EALINE_H
