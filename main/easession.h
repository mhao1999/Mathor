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

    
    // 获取几何元素
    EaPoint* getPoint(int pointId);
    EaLine* getLine(int lineId);
    const std::vector<std::shared_ptr<EaPoint>>& getPoints() const { return m_points; }
    const std::vector<std::shared_ptr<EaLine>>& getLines() const { return m_lines; }

    
    // 选择管理
    void selectPoint(int pointId, bool selected = true);
    void selectLine(int lineId, bool selected = true);
    void clearSelection();
    std::vector<int> getSelectedPoints() const;
    std::vector<int> getSelectedLines() const;
    
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
    void addDistanceConstraint(int point1Id, int point2Id, double distance);
    void createFixPointConstraint(int pointId);
    void addParallelConstraint(int line1Id, int line2Id);
    void addPtOnLineConstraint(int pointId, int lineId);
    void clear();
    void createConstraint1();
    void createGongdianConstraint();
    void createParallelConstraint();
    void createPtInLineConstraint();
    // 更新几何元素
    void updatePointPosition(int pointId, double x, double y, double z = 0.0);

signals:
    void geometryChanged();
    void pointAdded(int pointId, double x, double y, double z);
    void lineAdded(int lineId, int startPointId, int endPointId);
    void pointRemoved(int pointId);
    void lineRemoved(int lineId);
    void pointPositionChanged(int pointId, double x, double y, double z);
    void selectionChanged();

private:
    EaSession();

    // 几何元素存储
    std::vector<std::shared_ptr<EaPoint>> m_points;
    std::vector<std::shared_ptr<EaLine>> m_lines;
    
    // 约束存储,
    std::vector<Constraint> m_constraints;
    
    // ID管理
    int m_nextPointId = 1;
    int m_nextLineId = 1;
    int m_nextConstraintId = 1;
    
    // 选择状态
    std::vector<int> m_selectedPoints;
    std::vector<int> m_selectedLines;
    
    // GeometrySolver引用
    GeometrySolver* m_geometrySolver;

private:
    static EaSession *instance;
};

#endif // EASESSION_H
