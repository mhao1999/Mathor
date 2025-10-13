#include "easession.h"
#include "GeometrySolver.h"
#include <QDebug>
#include <algorithm>

EaSession *EaSession::instance = nullptr;

EaSession::EaSession() : QObject(), m_geometrySolver(nullptr)
{
}

EaSession::~EaSession() 
{
    clear();
}

// ============ 几何元素管理 ============

int EaSession::addPoint(double x, double y, double z)
{
    auto point = std::make_shared<EaPoint>();
    point->setPosition(x, y, z);
    point->setId(m_nextPointId);
    
    m_points.append(point);
    int pointId = m_nextPointId++;
    
    emit pointAdded(pointId, x, y, z);
    emit geometryChanged();
    
    qDebug() << "EaSession: Added point" << pointId << "at" << x << y << z;
    return pointId;
}

int EaSession::addLine(int startPointId, int endPointId)
{
    // 验证起点和终点是否存在
    EaPoint* startPoint = getPoint(startPointId);
    EaPoint* endPoint = getPoint(endPointId);
    
    if (!startPoint || !endPoint) {
        qWarning() << "EaSession: Cannot create line - invalid point IDs:" << startPointId << endPointId;
        return -1;
    }
    
    auto line = std::make_shared<EaLine>();
    line->setStartPoint(startPoint);
    line->setEndPoint(endPoint);
    line->setId(m_nextLineId);
    
    m_lines.append(line);
    int lineId = m_nextLineId++;
    
    emit lineAdded(lineId, startPointId, endPointId);
    emit geometryChanged();
    
    qDebug() << "EaSession: Added line" << lineId << "from point" << startPointId << "to" << endPointId;
    return lineId;
}

void EaSession::removePoint(int pointId)
{
    // 移除点
    auto it = std::find_if(m_points.begin(), m_points.end(),
                          [pointId](const std::shared_ptr<EaPoint>& point) {
                              return point->getId() == pointId;
                          });
    
    if (it != m_points.end()) {
        m_points.erase(it);
        emit pointRemoved(pointId);
        emit geometryChanged();
        qDebug() << "EaSession: Removed point" << pointId;
    }
    
    // 移除相关的线
    m_lines.erase(std::remove_if(m_lines.begin(), m_lines.end(),
                                [pointId](const std::shared_ptr<EaLine>& line) {
                                    return line->getStartPointId() == pointId || 
                                           line->getEndPointId() == pointId;
                                }), m_lines.end());
}

void EaSession::removeLine(int lineId)
{
    auto it = std::find_if(m_lines.begin(), m_lines.end(),
                          [lineId](const std::shared_ptr<EaLine>& line) {
                              return line->getId() == lineId;
                          });
    
    if (it != m_lines.end()) {
        m_lines.erase(it);
        emit lineRemoved(lineId);
        emit geometryChanged();
        qDebug() << "EaSession: Removed line" << lineId;
    }
}

void EaSession::clear()
{
    m_points.clear();
    m_lines.clear();
    m_constraints.clear();
    m_selectedPoints.clear();
    m_selectedLines.clear();
    m_nextPointId = 1;
    m_nextLineId = 1;
    m_nextConstraintId = 1;
    
    emit geometryChanged();
    emit selectionChanged();
    qDebug() << "EaSession: Cleared all geometry";
}

// ============ 获取几何元素 ============

EaPoint* EaSession::getPoint(int pointId)
{
    auto it = std::find_if(m_points.begin(), m_points.end(),
                          [pointId](const std::shared_ptr<EaPoint>& point) {
                              return point->getId() == pointId;
                          });
    
    return (it != m_points.end()) ? it->get() : nullptr;
}

EaLine* EaSession::getLine(int lineId)
{
    auto it = std::find_if(m_lines.begin(), m_lines.end(),
                          [lineId](const std::shared_ptr<EaLine>& line) {
                              return line->getId() == lineId;
                          });
    
    return (it != m_lines.end()) ? it->get() : nullptr;
}

// ============ 更新几何元素 ============

void EaSession::updatePointPosition(int pointId, double x, double y, double z)
{
    EaPoint* point = getPoint(pointId);
    if (point) {
        point->setPosition(x, y, z);
        emit pointPositionChanged(pointId, x, y, z);
        emit geometryChanged();
        qDebug() << "EaSession: Updated point" << pointId << "position to" << x << y << z;
    }
}

// ============ 选择管理 ============

void EaSession::selectPoint(int pointId, bool selected)
{
    if (selected) {
        if (!m_selectedPoints.contains(pointId)) {
            m_selectedPoints.append(pointId);
            emit selectionChanged();
        }
    } else {
        if (m_selectedPoints.removeOne(pointId)) {
            emit selectionChanged();
        }
    }
}

void EaSession::selectLine(int lineId, bool selected)
{
    if (selected) {
        if (!m_selectedLines.contains(lineId)) {
            m_selectedLines.append(lineId);
            emit selectionChanged();
        }
    } else {
        if (m_selectedLines.removeOne(lineId)) {
            emit selectionChanged();
        }
    }
}

void EaSession::clearSelection()
{
    if (!m_selectedPoints.isEmpty() || !m_selectedLines.isEmpty()) {
        m_selectedPoints.clear();
        m_selectedLines.clear();
        emit selectionChanged();
    }
}

QVector<int> EaSession::getSelectedPoints() const
{
    return m_selectedPoints;
}

QVector<int> EaSession::getSelectedLines() const
{
    return m_selectedLines;
}

// ============ 约束管理 ============

void EaSession::addDistanceConstraint(int point1Id, int point2Id, double distance)
{
    qDebug() << "EaSession: addDistanceConstraint called for points" << point1Id << point2Id << "distance" << distance;
    qDebug() << "EaSession: Current number of points:" << m_points.size();
    qDebug() << "EaSession: Current number of constraints:" << m_constraints.size();
    
    // 打印所有点的信息
    for (const auto& point : m_points) {
        qDebug() << "EaSession: Available point ID:" << point->getId() << "at" << point->pos().x() << point->pos().y();
    }
    
    // 验证点是否存在
    EaPoint* point1 = getPoint(point1Id);
    EaPoint* point2 = getPoint(point2Id);
    
    qDebug() << "EaSession: Point1 exists:" << (point1 != nullptr) << "Point2 exists:" << (point2 != nullptr);
    
    if (!point1 || !point2) {
        qWarning() << "EaSession: Cannot add distance constraint - invalid point IDs:" << point1Id << point2Id;
        qWarning() << "EaSession: Point1 exists:" << (point1 != nullptr) << "Point2 exists:" << (point2 != nullptr);
        qWarning() << "EaSession: Available point IDs:";
        for (const auto& point : m_points) {
            qWarning() << "EaSession:   - Point ID:" << point->getId();
        }
        return;
    }
    
    QVariantMap constraint;
    constraint["id"] = m_nextConstraintId++;
    constraint["type"] = "distance";
    constraint["point1"] = point1Id;
    constraint["point2"] = point2Id;
    constraint["distance"] = distance;
    
    m_constraints.append(constraint);
    
    qDebug() << "EaSession: Added distance constraint" << constraint["id"] 
             << "between points" << point1Id << "and" << point2Id 
             << "with distance" << distance;
    qDebug() << "EaSession: Total constraints after adding:" << m_constraints.size();
    qDebug() << "EaSession: Constraint details:" << constraint;
}

void EaSession::removeConstraint(int constraintId)
{
    for (int i = 0; i < m_constraints.size(); ++i) {
        QVariantMap constraint = m_constraints[i].toMap();
        if (constraint["id"].toInt() == constraintId) {
            m_constraints.removeAt(i);
            qDebug() << "EaSession: Removed constraint" << constraintId;
            return;
        }
    }
}

void EaSession::clearConstraints()
{
    m_constraints.clear();
    m_nextConstraintId = 1;
    qDebug() << "EaSession: Cleared all constraints";
}

QVariantList EaSession::getConstraints() const
{
    return m_constraints;
}

bool EaSession::solveDragConstraint(int draggedPointId, double newX, double newY)
{
    qDebug() << "EaSession: solveDragConstraint called for point" << draggedPointId 
             << "to position" << newX << newY;
    
    if (!m_geometrySolver) {
        qWarning() << "EaSession: No GeometrySolver available for constraint solving";
        return false;
    }
    
    qDebug() << "EaSession: Number of constraints:" << m_constraints.size();
    for (int i = 0; i < m_constraints.size(); ++i) {
        QVariantMap constraint = m_constraints[i].toMap();
        qDebug() << "EaSession: Constraint" << i << ":" << constraint;
    }
    
    qDebug() << "EaSession: Constraints being passed to GeometrySolver:" << m_constraints;
    
    // 构建点位置映射
    QVariantMap pointPositions;
    for (const auto& point : m_points) {
        QVariantMap pos;
        pos["x"] = point->pos().x();
        pos["y"] = point->pos().y();
        pos["z"] = point->pos().z();
        pointPositions[QString::number(point->getId())] = pos;
        qDebug() << "EaSession: Point" << point->getId() << "at" << pos;
    }
    
    // 调用GeometrySolver进行求解
    bool success = m_geometrySolver->solveDragConstraint(draggedPointId, newX, newY, 
                                                        pointPositions, m_constraints);
    
    if (success) {
        // 更新点的位置
        QVariantMap solvedPoints = m_geometrySolver->getSolvedPoints();
        
        // 更新所有点的位置
        for (const auto& point : m_points) {
            int pointId = point->getId();
            if (pointId == 1) {
                point->setPosition(solvedPoints["x1"].toDouble(), solvedPoints["y1"].toDouble(), 0.0);
            } else if (pointId == 2) {
                qDebug() << "new pt " << solvedPoints["x2"].toDouble() << " " <<
                    solvedPoints["y2"].toDouble();
                point->setPosition(solvedPoints["x2"].toDouble(), solvedPoints["y2"].toDouble(), 0.0);
            }
        }
        
        emit geometryChanged();
        qDebug() << "EaSession: Constraint solving successful for point" << draggedPointId;
    } else {
        qWarning() << "EaSession: Constraint solving failed for point" << draggedPointId;
    }
    
    return success;
}

void EaSession::setGeometrySolver(GeometrySolver* solver)
{
    m_geometrySolver = solver;
    qDebug() << "EaSession: GeometrySolver reference set";
}

void EaSession::testConstraints()
{
    qDebug() << "EaSession: Testing constraints...";
    qDebug() << "EaSession: Number of points:" << m_points.size();
    qDebug() << "EaSession: Number of constraints:" << m_constraints.size();
    qDebug() << "EaSession: Next constraint ID:" << m_nextConstraintId;
    
    for (int i = 0; i < m_constraints.size(); ++i) {
        QVariantMap constraint = m_constraints[i].toMap();
        qDebug() << "EaSession: Constraint" << i << ":" << constraint;
    }
    
    for (const auto& point : m_points) {
        qDebug() << "EaSession: Point" << point->getId() << "at" 
                 << point->pos().x() << point->pos().y() << point->pos().z();
    }
    
    if (m_geometrySolver) {
        qDebug() << "EaSession: GeometrySolver is available";
    } else {
        qDebug() << "EaSession: GeometrySolver is NOT available";
    }
    
    // 测试手动添加约束
    qDebug() << "EaSession: Testing manual constraint addition...";
    addDistanceConstraint(1, 2, 100.0);
    qDebug() << "EaSession: After manual addition, constraints:" << m_constraints.size();
}

bool EaSession::testDragConstraint(int pointId, double x, double y)
{
    qDebug() << "EaSession: Testing drag constraint for point" << pointId << "to" << x << y;
    return solveDragConstraint(pointId, x, y);
}
