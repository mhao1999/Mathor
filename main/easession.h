#ifndef EASESSION_H
#define EASESSION_H

#include <QObject>
#include <QVector>
#include <memory>
// #include "../geometry/eashape.h"
#include "../geometry/eapoint.h"
#include "../geometry/ealine.h"

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
    void clear();
    
    // 获取几何元素
    EaPoint* getPoint(int pointId);
    EaLine* getLine(int lineId);
    const QVector<std::shared_ptr<EaPoint>>& getPoints() const { return m_points; }
    const QVector<std::shared_ptr<EaLine>>& getLines() const { return m_lines; }

    
    // 选择管理
    void selectPoint(int pointId, bool selected = true);
    void selectLine(int lineId, bool selected = true);
    void clearSelection();
    QVector<int> getSelectedPoints() const;
    QVector<int> getSelectedLines() const;
    
    // 约束管理

    void removeConstraint(int constraintId);
    void clearConstraints();
    QVariantList getConstraints() const;
    
    // 拖拽约束求解
    bool solveDragConstraint(int draggedPointId, double newX, double newY);
    
    // 设置GeometrySolver引用
    void setGeometrySolver(GeometrySolver* solver);
    
    // 测试方法
    Q_INVOKABLE void testConstraints();
    Q_INVOKABLE bool testDragConstraint(int pointId, double x, double y);

public slots:
    // 几何元素管理
    int addPoint(double x, double y, double z = 0.0);
    int addLine(int startPointId, int endPointId);
    void addDistanceConstraint(int point1Id, int point2Id, double distance);
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
    QVector<std::shared_ptr<EaPoint>> m_points;
    QVector<std::shared_ptr<EaLine>> m_lines;
    
    // 约束存储
    QVariantList m_constraints;
    
    // ID管理
    int m_nextPointId = 1;
    int m_nextLineId = 1;
    int m_nextConstraintId = 1;
    
    // 选择状态
    QVector<int> m_selectedPoints;
    QVector<int> m_selectedLines;
    
    // GeometrySolver引用
    GeometrySolver* m_geometrySolver;

private:
    static EaSession *instance;
};

#endif // EASESSION_H
