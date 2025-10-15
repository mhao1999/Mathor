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
    
    // 添加固定点1的约束
    this->createFixPointConstraint(pt1);
    
    this->addDistanceConstraint(pt1, pt2, 100.0);
}

void EaSession::createGongdianConstraint()
{
    this->clear();
    
    // 添加四个点
    int pt1 = this->addPoint(-100.0, -120.0);
    int pt2 = this->addPoint(150.0, -120.0);
    int pt3 = this->addPoint(30.0, 150.0);
    int pt4 = this->addPoint(30.0, 30.0);  // 共点，可以自由移动
    
    // 添加三条线段：(点1，点4)，(点2，点4)，(点3，点4)
    this->addLine(pt1, pt4);
    this->addLine(pt2, pt4);
    this->addLine(pt3, pt4);
    
    // 为点1、点2、点3添加固定约束
    this->createFixPointConstraint(pt1);
    this->createFixPointConstraint(pt2);
    this->createFixPointConstraint(pt3);
    
    qDebug() << "EaSession: Created gongdian constraint with points" << pt1 << pt2 << pt3 << pt4;
    qDebug() << "EaSession: Points" << pt1 << pt2 << pt3 << "are fixed, point" << pt4 << "can move freely";
}

void EaSession::createParallelConstraint()
{
    this->clear();
    
    // 创建4个点，构成两条线段
    // 第一条线段：(点1，点2)
    int pt1 = this->addPoint(50.0, 50.0);   // 线段1起点
    int pt2 = this->addPoint(150.0, 100.0); // 线段1终点
    
    // 第二条线段：(点3，点4)
    int pt3 = this->addPoint(50.0, 150.0);  // 线段2起点
    int pt4 = this->addPoint(150.0, 200.0); // 线段2终点
    
    // 创建两条线段
    int line1 = this->addLine(pt1, pt2);    // 线段1
    int line2 = this->addLine(pt3, pt4);    // 线段2
    
    this->createFixPointConstraint(pt1);
    // this->createFixPointConstraint(pt2);
    this->createFixPointConstraint(pt3);
    this->createFixPointConstraint(pt4);
    
    // 为两条线段添加平行约束
    this->addParallelConstraint(line1, line2);
    
    qDebug() << "EaSession: Created parallel constraint with points" << pt1 << pt2 << pt3 << pt4;
    qDebug() << "EaSession: Created lines" << line1 << "and" << line2 << "with parallel constraint";
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

void EaSession::createFixPointConstraint(int pointId)
{
    // 添加固定点的约束
    Constraint fixConstraint(m_nextConstraintId++, "fix_point");
    fixConstraint.data["point"] = pointId;
    fixConstraint.data["type"] = "SLVS_C_WHERE_DRAGGED";
    
    m_constraints.push_back(fixConstraint);
    
    qDebug() << "EaSession: Added fix point constraint" << fixConstraint.id 
             << "for point" << pointId;
}


void EaSession::addParallelConstraint(int line1Id, int line2Id)
{
    // 验证线段是否存在
    EaLine* line1 = getLine(line1Id);
    EaLine* line2 = getLine(line2Id);
    
    if (!line1 || !line2) {
        qWarning() << "EaSession: Cannot add parallel constraint - invalid line IDs:" << line1Id << line2Id;
        return;
    }
    
    // 添加平行约束
    Constraint parallelConstraint(m_nextConstraintId++, "parallel");
    parallelConstraint.data["line1"] = line1Id;
    parallelConstraint.data["line2"] = line2Id;
    
    m_constraints.push_back(parallelConstraint);
    
    qDebug() << "EaSession: Added parallel constraint" << parallelConstraint.id 
             << "between lines" << line1Id << "and" << line2Id;
}

void EaSession::addPtOnLineConstraint(int pointId, int lineId)
{
    // 验证点和线段是否存在
    EaPoint* point = getPoint(pointId);
    EaLine* line = getLine(lineId);
    
    if (!point || !line) {
        qWarning() << "EaSession: Cannot add point on line constraint - invalid point or line IDs:" << pointId << lineId;
        return;
    }
    
    // 添加点在线上约束
    Constraint ptOnLineConstraint(m_nextConstraintId++, "pt_on_line");
    ptOnLineConstraint.data["point"] = pointId;
    ptOnLineConstraint.data["line"] = lineId;
    
    m_constraints.push_back(ptOnLineConstraint);
    
    qDebug() << "EaSession: Added point on line constraint" << ptOnLineConstraint.id 
             << "for point" << pointId << "on line" << lineId;
}

void EaSession::createPtInLineConstraint()
{
    this->clear();
    
    // 创建3个点
    int pt1 = this->addPoint(50.0, 50.0);   // 线段起点
    int pt2 = this->addPoint(150.0, 100.0); // 线段终点
    int pt3 = this->addPoint(100.0, 75.0);  // 在线段上的点
    
    // 创建线段（点1，点2）
    int line1 = this->addLine(pt1, pt2);
    
    // 为点1和点2添加固定约束，使线段保持稳定
    this->createFixPointConstraint(pt1);
    this->createFixPointConstraint(pt2);
    
    // 为点3添加点在线上约束
    this->addPtOnLineConstraint(pt3, line1);
    
    qDebug() << "EaSession: Created point on line constraint with points" << pt1 << pt2 << pt3;
    qDebug() << "EaSession: Created line" << line1 << "with point" << pt3 << "on it";
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
    
    // 构建线段信息映射
    std::map<std::string, std::map<std::string, std::any>> lineInfo;
    for (const auto& line : m_lines) {
        std::map<std::string, std::any> lineData;
        lineData["startPoint"] = line->getStartPointId();
        lineData["endPoint"] = line->getEndPointId();
        lineInfo[std::to_string(line->getId())] = lineData;
    }
    
    // 调用GeometrySolver进行求解
    bool success = m_geometrySolver->solveDragConstraint(draggedPointId, newX, newY, 
                                                        pointPositions, m_constraints, lineInfo);
    
    if (success) {
        // 更新点的位置 - 使用动态方法处理所有点
        QVariantMap solvedPoints = m_geometrySolver->getSolvedPoints(pointPositions);
        
        // 更新所有点的位置
        for (const auto& point : m_points) {
            int pointId = point->getId();
            QString xKey = QString("x%1").arg(pointId);
            QString yKey = QString("y%1").arg(pointId);
            
            if (solvedPoints.contains(xKey) && solvedPoints.contains(yKey)) {
                double newX = solvedPoints[xKey].toDouble();
                double newY = solvedPoints[yKey].toDouble();
                
                point->setPosition(newX, newY, 0.0);
                
                qDebug() << "EaSession: Updated point" << pointId << "to position" << newX << newY;
            } else {
                qWarning() << "EaSession: No solved position found for point" << pointId;
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

