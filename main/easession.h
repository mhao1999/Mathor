#ifndef EASESSION_H
#define EASESSION_H

#include <QObject>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <any>
// #include "../geometry/eashape.h"
#include "../geometry/eapoint.h"
#include "../geometry/ealine.h"
#include "../geometry/eacircle.h"

// 约束结构体，替代QVariantMap
struct Constraint {
    int id;
    std::string type;
    std::map<std::string, std::any> data;
    
    Constraint() : id(0) {}
    Constraint(int id, const std::string& type) : id(id), type(type) {}
};

class GeometrySolver;

class EaSession : public QObject
{
    Q_OBJECT

public:
    static EaSession *getInstance() {
        if (!instance) {
            instance = new EaSession;
        }
        return instance;
    }

    ~EaSession();

    // 几何元素管理
    void removePoint(int pointId);
    void removeLine(int lineId);
    void removeCircle(int circleId);

    
    // 获取几何元素
    EaPoint* getPoint(int pointId);
    EaLine* getLine(int lineId);
    EaCircle* getCircle(int circleId);
    const std::vector<std::shared_ptr<EaPoint>>& getPoints() const { return m_points; }
    const std::vector<std::shared_ptr<EaLine>>& getLines() const { return m_lines; }
    const std::vector<std::shared_ptr<EaCircle>>& getCircles() const { return m_circles; }

    
    // 选择管理
    void selectPoint(int pointId, bool selected = true);
    void selectLine(int lineId, bool selected = true);
    void selectCircle(int circleId, bool selected = true);
    void clearSelection();
    std::vector<int> getSelectedPoints() const;
    std::vector<int> getSelectedLines() const;
    std::vector<int> getSelectedCircles() const;
    
    // 约束管理

    void removeConstraint(int constraintId);
    void clearConstraints();
    std::vector<Constraint> getConstraints() const;
    
    // 拖拽约束求解
    bool solveDragConstraint(int draggedPointId, double newX, double newY);
    
    // 设置GeometrySolver引用
    void setGeometrySolver(GeometrySolver* solver);
    
public slots:
    // 几何元素管理
    int addPoint(double x, double y, double z = 0.0);
    int addLine(int startPointId, int endPointId);
    int addCircle(int centerPointId, double radius);
    void addDistanceConstraint(int point1Id, int point2Id, double distance);
    void createFixPointConstraint(int pointId);
    void addParallelConstraint(int line1Id, int line2Id);
    void addPerpendicularConstraint(int line1Id, int line2Id);
    void addHorizontalConstraint(int lineId);
    void addVerticalConstraint(int lineId);
    void addPtOnLineConstraint(int pointId, int lineId);
    void addPtOnCircleConstraint(int pointId, int centerPointId, double radius);
    void clear();
    void createConstraint1();
    void createGongdianConstraint();
    void createParallelConstraint();
    void createPtInLineConstraint();
    void createPtOnCircleConstraint();
    void createPerpendicularConstraint();
    void createHorizontalConstraint();
    void createVerticalConstraint();
    // 更新几何元素
    void updatePointPosition(int pointId, double x, double y, double z = 0.0);

signals:
    void geometryChanged();
    void pointAdded(int pointId, double x, double y, double z);
    void lineAdded(int lineId, int startPointId, int endPointId);
    void circleAdded(int circleId, int centerPointId, double radius);
    void pointRemoved(int pointId);
    void lineRemoved(int lineId);
    void circleRemoved(int circleId);
    void pointPositionChanged(int pointId, double x, double y, double z);
    void selectionChanged();

private:
    EaSession();

    // 几何元素存储
    std::vector<std::shared_ptr<EaPoint>> m_points;
    std::vector<std::shared_ptr<EaLine>> m_lines;
    std::vector<std::shared_ptr<EaCircle>> m_circles;
    
    // 约束存储,
    std::vector<Constraint> m_constraints;
    
    // ID管理
    int m_nextPointId = 1;
    int m_nextLineId = 1;
    int m_nextCircleId = 1;
    int m_nextConstraintId = 1;
    
    // 选择状态
    std::vector<int> m_selectedPoints;
    std::vector<int> m_selectedLines;
    std::vector<int> m_selectedCircles;
    
    // GeometrySolver引用
    GeometrySolver* m_geometrySolver;

private:
    static EaSession *instance;
};

#endif // EASESSION_H
