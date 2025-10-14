#include "easession.h"
#include "eageosolver.h"
#include <QDebug>
#include <QVariantMap>
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
    
    m_points.push_back(point);
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
    
    m_lines.push_back(line);
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

void EaSession::createConstraint1()
{
    this->clear();
    int pt1 = this->addPoint(10.0, 20.0);
    int pt2 = this->addPoint(50.0, 60.0);
    this->addLine(pt1, pt2);
    this->addDistanceConstraint(pt1, pt2, 100.0);
}

void EaSession::createGongdianConstraint()
{

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
        auto it = std::find(m_selectedPoints.begin(), m_selectedPoints.end(), pointId);
        if (it == m_selectedPoints.end()) {
            m_selectedPoints.push_back(pointId);
            emit selectionChanged();
        }
    } else {
        auto it = std::find(m_selectedPoints.begin(), m_selectedPoints.end(), pointId);
        if (it != m_selectedPoints.end()) {
            m_selectedPoints.erase(it);
            emit selectionChanged();
        }
    }
}

void EaSession::selectLine(int lineId, bool selected)
{
    if (selected) {
        auto it = std::find(m_selectedLines.begin(), m_selectedLines.end(), lineId);
        if (it == m_selectedLines.end()) {
            m_selectedLines.push_back(lineId);
            emit selectionChanged();
        }
    } else {
        auto it = std::find(m_selectedLines.begin(), m_selectedLines.end(), lineId);
        if (it != m_selectedLines.end()) {
            m_selectedLines.erase(it);
            emit selectionChanged();
        }
    }
}

void EaSession::clearSelection()
{
    if (!m_selectedPoints.empty() || !m_selectedLines.empty()) {
        m_selectedPoints.clear();
        m_selectedLines.clear();
        emit selectionChanged();
    }
}

std::vector<int> EaSession::getSelectedPoints() const
{
    return m_selectedPoints;
}

std::vector<int> EaSession::getSelectedLines() const
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
    
    Constraint constraint(m_nextConstraintId++, "distance");
    constraint.data["point1"] = point1Id;
    constraint.data["point2"] = point2Id;
    constraint.data["distance"] = distance;
    
    m_constraints.push_back(constraint);
    
    qDebug() << "EaSession: Added distance constraint" << constraint.id 
             << "between points" << point1Id << "and" << point2Id 
             << "with distance" << distance;
    qDebug() << "EaSession: Total constraints after adding:" << m_constraints.size();
}

void EaSession::removeConstraint(int constraintId)
{
    auto it = std::find_if(m_constraints.begin(), m_constraints.end(),
                          [constraintId](const Constraint& constraint) {
                              return constraint.id == constraintId;
                          });
    if (it != m_constraints.end()) {
        m_constraints.erase(it);
        qDebug() << "EaSession: Removed constraint" << constraintId;
    }
}

void EaSession::clearConstraints()
{
    m_constraints.clear();
    m_nextConstraintId = 1;
    qDebug() << "EaSession: Cleared all constraints";
}

std::vector<Constraint> EaSession::getConstraints() const
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
    for (size_t i = 0; i < m_constraints.size(); ++i) {
        const Constraint& constraint = m_constraints[i];
        qDebug() << "EaSession: Constraint" << i << ":" << constraint.id << constraint.type.c_str();
    }
    
    // 构建点位置映射
    std::map<std::string, std::map<std::string, std::any>> pointPositions;
    for (const auto& point : m_points) {
        std::map<std::string, std::any> pos;
        pos["x"] = point->pos().x();
        pos["y"] = point->pos().y();
        pos["z"] = point->pos().z();
        pointPositions[std::to_string(point->getId())] = pos;
        qDebug() << "EaSession: Point" << point->getId() << "at" << point->pos().x() << point->pos().y() << point->pos().z();
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
                qDebug() << "new pt " << solvedPoints["x2"].toDouble() << " " << solvedPoints["y2"].toDouble();
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

