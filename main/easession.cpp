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
    
    // 添加到分类容器
    m_points.push_back(point);
    // 添加到统一容器
    m_shapes.push_back(std::static_pointer_cast<EaShape>(point));
    
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
    
    // 添加到分类容器
    m_lines.push_back(line);
    // 添加到统一容器
    m_shapes.push_back(std::static_pointer_cast<EaShape>(line));
    
    int lineId = m_nextLineId++;
    
    emit lineAdded(lineId, startPointId, endPointId);
    emit geometryChanged();
    
    qDebug() << "EaSession: Added line" << lineId << "from point" << startPointId << "to" << endPointId;
    return lineId;
}

int EaSession::addCircle(int centerPointId, double radius)
{
    // 验证圆心是否存在
    EaPoint* centerPoint = getPoint(centerPointId);
    
    if (!centerPoint) {
        qWarning() << "EaSession: Cannot create circle - invalid center point ID:" << centerPointId;
        return -1;
    }
    
    auto circle = std::make_shared<EaCircle>();
    circle->setCenter(centerPoint);
    circle->setRadius(radius);
    circle->setId(m_nextCircleId);
    
    // 添加到分类容器
    m_circles.push_back(circle);
    // 添加到统一容器
    m_shapes.push_back(std::static_pointer_cast<EaShape>(circle));
    
    int circleId = m_nextCircleId++;
    
    emit circleAdded(circleId, centerPointId, radius);
    emit geometryChanged();
    
    qDebug() << "EaSession: Added circle" << circleId << "with center point" << centerPointId << "radius" << radius;
    return circleId;
}

int EaSession::addArc(int centerPointId, double radius, double start, double end)
{
    // 验证圆心是否存在
    EaPoint* centerPoint = getPoint(centerPointId);

    if (!centerPoint) {
        qWarning() << "EaSession: Cannot create arc - invalid center point ID:" << centerPointId;
        return -1;
    }

    auto arc = std::make_shared<EaArc>();
    arc->setCenter(centerPoint);
    arc->setRadius(radius);
    arc->setStartAngle(start);
    arc->setEndAngle(end);
    arc->setId(m_nextCircleId); // 使用相同的ID计数器

    // 添加到分类容器
    m_arcs.push_back(arc);
    // 添加到统一容器
    m_shapes.push_back(std::static_pointer_cast<EaShape>(arc));

    int arcId = m_nextCircleId++;

    emit arcAdded(arcId, centerPointId, radius, start, end);
    emit geometryChanged();

    qDebug() << "EaSession: Added arc" << arcId << "with center point" << centerPointId << "radius" << radius;
    return arcId;
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

void EaSession::removeCircle(int circleId)
{
    auto it = std::find_if(m_circles.begin(), m_circles.end(),
                          [circleId](const std::shared_ptr<EaCircle>& circle) {
                              return circle->getId() == circleId;
                          });
    
    if (it != m_circles.end()) {
        m_circles.erase(it);
        emit circleRemoved(circleId);
        emit geometryChanged();
        qDebug() << "EaSession: Removed circle" << circleId;
    }
}

void EaSession::clear()
{
    // 清空统一容器
    m_shapes.clear();
    
    // 清空分类容器
    m_points.clear();
    m_lines.clear();
    m_circles.clear();
    m_arcs.clear();
    m_constraints.clear();
    m_selectedPoints.clear();
    m_selectedLines.clear();
    m_selectedCircles.clear();
    m_nextPointId = 1;
    m_nextLineId = 1;
    m_nextCircleId = 1;
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

EaCircle* EaSession::getCircle(int circleId)
{
    auto it = std::find_if(m_circles.begin(), m_circles.end(),
                          [circleId](const std::shared_ptr<EaCircle>& circle) {
                              return circle->getId() == circleId;
                          });
    
    return (it != m_circles.end()) ? it->get() : nullptr;
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

void EaSession::selectCircle(int circleId, bool selected)
{
    if (selected) {
        auto it = std::find(m_selectedCircles.begin(), m_selectedCircles.end(), circleId);
        if (it == m_selectedCircles.end()) {
            m_selectedCircles.push_back(circleId);
            emit selectionChanged();
        }
    } else {
        auto it = std::find(m_selectedCircles.begin(), m_selectedCircles.end(), circleId);
        if (it != m_selectedCircles.end()) {
            m_selectedCircles.erase(it);
            emit selectionChanged();
        }
    }
}

void EaSession::clearSelection()
{
    if (!m_selectedPoints.empty() || !m_selectedLines.empty() || !m_selectedCircles.empty()) {
        m_selectedPoints.clear();
        m_selectedLines.clear();
        m_selectedCircles.clear();
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

std::vector<int> EaSession::getSelectedCircles() const
{
    return m_selectedCircles;
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

void EaSession::addPerpendicularConstraint(int line1Id, int line2Id)
{
    // 验证线段是否存在
    EaLine* line1 = getLine(line1Id);
    EaLine* line2 = getLine(line2Id);
    
    if (!line1 || !line2) {
        qWarning() << "EaSession: Cannot add perpendicular constraint - invalid line IDs:" << line1Id << line2Id;
        return;
    }
    
    // 添加垂直约束
    Constraint perpendicularConstraint(m_nextConstraintId++, "perpendicular");
    perpendicularConstraint.data["line1"] = line1Id;
    perpendicularConstraint.data["line2"] = line2Id;
    
    m_constraints.push_back(perpendicularConstraint);
    
    qDebug() << "EaSession: Added perpendicular constraint" << perpendicularConstraint.id 
             << "between lines" << line1Id << "and" << line2Id;
}

void EaSession::addHorizontalConstraint(int lineId)
{
    // 验证线段是否存在
    EaLine* line = getLine(lineId);
    
    if (!line) {
        qWarning() << "EaSession: Cannot add horizontal constraint - invalid line ID:" << lineId;
        return;
    }
    
    // 添加水平约束
    Constraint horizontalConstraint(m_nextConstraintId++, "horizontal");
    horizontalConstraint.data["line"] = lineId;
    
    m_constraints.push_back(horizontalConstraint);
    
    qDebug() << "EaSession: Added horizontal constraint" << horizontalConstraint.id 
             << "for line" << lineId;
}

void EaSession::addVerticalConstraint(int lineId)
{
    // 验证线段是否存在
    EaLine* line = getLine(lineId);

    if (!line) {
        qWarning() << "EaSession: Cannot add vertical constraint - invalid line ID:" << lineId;
        return;
    }

    // 添加水平约束
    Constraint verticalConstraint(m_nextConstraintId++, "vertical");
    verticalConstraint.data["line"] = lineId;

    m_constraints.push_back(verticalConstraint);

    qDebug() << "EaSession: Added vertical constraint" << verticalConstraint.id
             << "for line" << lineId;
}

void EaSession::addAngleConstraint(int line1Id, int line2Id, double angle)
{
    // 验证线段是否存在
    EaLine* line1 = getLine(line1Id);
    EaLine* line2 = getLine(line2Id);
    
    if (!line1 || !line2) {
        qWarning() << "EaSession: Cannot add angle constraint - invalid line IDs:" << line1Id << line2Id;
        return;
    }
    
    // 添加角度约束
    Constraint angleConstraint(m_nextConstraintId++, "angle");
    angleConstraint.data["line1"] = line1Id;
    angleConstraint.data["line2"] = line2Id;
    angleConstraint.data["angle"] = angle;
    
    m_constraints.push_back(angleConstraint);
    
    qDebug() << "EaSession: Added angle constraint" << angleConstraint.id 
             << "between lines" << line1Id << "and" << line2Id << "with angle" << angle << "degrees";
}

void EaSession::addArcLineTangentConstraint(int arcId, int lineId)
{
    // 验证圆弧和直线是否存在
    // 注意：这里我们需要通过统一容器来获取圆弧，因为EaArc没有单独的getArc方法
    EaLine* line = getLine(lineId);
    
    if (!line) {
        qWarning() << "EaSession: Cannot add arc-line tangent constraint - invalid line ID:" << lineId;
        return;
    }
    
    // 验证圆弧是否存在（通过统一容器查找）
    bool arcFound = false;
    for (const auto& shape : m_shapes) {
        if (auto arc = std::dynamic_pointer_cast<EaArc>(shape)) {
            if (arc->getId() == arcId) {
                arcFound = true;
                break;
            }
        }
    }
    
    if (!arcFound) {
        qWarning() << "EaSession: Cannot add arc-line tangent constraint - invalid arc ID:" << arcId;
        return;
    }
    
    // 添加圆弧与直线相切约束
    Constraint tangentConstraint(m_nextConstraintId++, "arc_line_tangent");
    tangentConstraint.data["arc"] = arcId;
    tangentConstraint.data["line"] = lineId;
    
    m_constraints.push_back(tangentConstraint);
    
    qDebug() << "EaSession: Added arc-line tangent constraint" << tangentConstraint.id 
             << "between arc" << arcId << "and line" << lineId;
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

void EaSession::addPtOnCircleConstraint(int pointId, int centerPointId, double radius)
{
    // 验证点是否存在
    EaPoint* point = getPoint(pointId);
    EaPoint* centerPoint = getPoint(centerPointId);
    
    if (!point || !centerPoint) {
        qWarning() << "EaSession: Cannot add point on circle constraint - invalid point IDs:" << pointId << centerPointId;
        return;
    }
    
    // 添加点在圆上约束
    Constraint ptOnCircleConstraint(m_nextConstraintId++, "pt_on_circle");
    ptOnCircleConstraint.data["point"] = pointId;
    ptOnCircleConstraint.data["center"] = centerPointId;
    ptOnCircleConstraint.data["radius"] = radius;
    
    m_constraints.push_back(ptOnCircleConstraint);
    
    qDebug() << "EaSession: Added point on circle constraint" << ptOnCircleConstraint.id 
             << "for point" << pointId << "on circle with center" << centerPointId << "radius" << radius;
}

void EaSession::addSymmetricLineConstraint(int point1Id, int point2Id, int lineId)
{
    // 验证点和线段是否存在
    EaPoint* point1 = getPoint(point1Id);
    EaPoint* point2 = getPoint(point2Id);
    EaLine* line = getLine(lineId);
    
    if (!point1 || !point2 || !line) {
        qWarning() << "EaSession: Cannot add symmetric line constraint - invalid point or line IDs:" << point1Id << point2Id << lineId;
        return;
    }
    
    // 添加关于直线对称的约束
    Constraint symmetricLineConstraint(m_nextConstraintId++, "symmetric_line");
    symmetricLineConstraint.data["point1"] = point1Id;
    symmetricLineConstraint.data["point2"] = point2Id;
    symmetricLineConstraint.data["line"] = lineId;
    symmetricLineConstraint.data["type"] = "SLVS_C_SYMMETRIC_LINE";
    
    m_constraints.push_back(symmetricLineConstraint);
    
    qDebug() << "EaSession: Added symmetric line constraint" << symmetricLineConstraint.id 
             << "for points" << point1Id << "and" << point2Id << "about line" << lineId;
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

void EaSession::createPtOnCircleConstraint()
{
    this->clear();
    
    // 创建圆心点
    int centerPt = this->addPoint(100.0, 100.0);  // 圆心位置
    
    // 创建圆上的点
    int ptOnCircle = this->addPoint(130.0, 100.0); // 圆上的点，初始在圆心右侧30单位处
    
    // 创建圆实体用于界面显示
    int circleId = this->addCircle(centerPt, 130.0); // 半径30.0
    
    // 为圆心添加固定约束，使圆保持稳定
    this->createFixPointConstraint(centerPt);
    
    // 添加点在圆上约束
    this->addPtOnCircleConstraint(ptOnCircle, centerPt, 130.0); // 半径30.0
    
    qDebug() << "EaSession: Created point on circle constraint with center point" << centerPt 
             << "and point on circle" << ptOnCircle << "with radius 130.0";
    qDebug() << "EaSession: Created circle" << circleId << "for display";
}

void EaSession::createPerpendicularConstraint()
{
    this->clear();
    
    // 创建4个点，构成两条垂直的线段
    // 第一条线段：(点1，点2) - 水平线段
    int pt1 = this->addPoint(50.0, 100.0);   // 线段1起点
    int pt2 = this->addPoint(150.0, 100.0);  // 线段1终点
    
    // 第二条线段：(点3，点4) - 垂直线段
    int pt3 = this->addPoint(100.0, 50.0);   // 线段2起点
    int pt4 = this->addPoint(100.0, 150.0);  // 线段2终点
    
    // 创建两条线段
    int line1 = this->addLine(pt1, pt2);     // 水平线段
    int line2 = this->addLine(pt3, pt4);     // 垂直线段
    
    // 固定一些点以稳定约束系统
    // this->createFixPointConstraint(pt1);     // 固定线段1起点
    this->createFixPointConstraint(pt3);     // 固定线段2起点
    this->createFixPointConstraint(pt4);
    
    // 添加垂直约束
    this->addPerpendicularConstraint(line1, line2);
    
    qDebug() << "EaSession: Created perpendicular constraint between line" << line1 << "and line" << line2;
}

void EaSession::createHorizontalConstraint()
{
    this->clear();
    
    // 创建一条线段，初始不是水平的
    int pt1 = this->addPoint(50.0, 80.0);    // 线段起点
    int pt2 = this->addPoint(150.0, 120.0);  // 线段终点（不是水平）
    
    // 创建线段
    int line1 = this->addLine(pt1, pt2);
    
    // 固定一个点以稳定约束系统
    // this->createFixPointConstraint(pt1);     // 固定线段起点
    
    // 添加水平约束
    this->addHorizontalConstraint(line1);
    
    qDebug() << "EaSession: Created horizontal constraint for line" << line1;
}

void EaSession::createVerticalConstraint()
{
    this->clear();

    // 创建一条线段，初始不是水平的
    int pt1 = this->addPoint(50.0, 80.0);    // 线段起点
    int pt2 = this->addPoint(150.0, 120.0);  // 线段终点（不是水平）

    // 创建线段
    int line1 = this->addLine(pt1, pt2);

    // 固定一个点以稳定约束系统
    // this->createFixPointConstraint(pt1);     // 固定线段起点

    // 添加水平约束
    this->addVerticalConstraint(line1);

    qDebug() << "EaSession: Created vertical constraint for line" << line1;
}

void EaSession::createAngleConstraint()
{
    this->clear();
    
    // 创建4个点，构成两条线段
    // 第一条线段：(点1，点2) - 水平线段
    int pt1 = this->addPoint(50.0, 100.0);   // 线段1起点
    int pt2 = this->addPoint(150.0, 100.0);  // 线段1终点
    
    // 第二条线段：(点3，点4) - 与第一条线段成45度角
    int pt3 = this->addPoint(100.0, 50.0);   // 线段2起点
    int pt4 = this->addPoint(150.0, 150.0);  // 线段2终点
    
    // 创建两条线段
    int line1 = this->addLine(pt1, pt2);     // 水平线段
    int line2 = this->addLine(pt3, pt4);     // 斜线段
    
    // 固定一些点以稳定约束系统
    this->createFixPointConstraint(pt1);     // 固定线段1起点
    this->createFixPointConstraint(pt3);     // 固定线段2起点
    this->addHorizontalConstraint(line2);
    
    // 添加角度约束（45度角）
    this->addAngleConstraint(line1, line2, 45.0);
    
    qDebug() << "EaSession: Created angle constraint between line" << line1 << "and line" << line2 << "with angle 45 degrees";
}

void EaSession::createLineTangentConstraint()
{
    this->clear();

    // 创建圆心点
    int centerPt = this->addPoint(0.0, 0.0);  // 圆心位置

    // 创建直线：两个点构成一条与圆弧相切的直线
    // 计算相切点位置：圆弧在0度时，相切点应该在(230, 100)
    int pt1 = this->addPoint(230.0, 100.0);   // 直线起点（相切点）
    int pt2 = this->addPoint(300.0, 100.0);  // 直线终点

    // 创建直线
    int lineId = this->addLine(pt1, pt2);

    // 创建圆弧实体用于界面显示
    int arcId = this->addArc(centerPt, 130.0, 0.0, 90.0); // 半径130.0，从0度到90度

    // 固定一些点以稳定约束系统
    this->createFixPointConstraint(centerPt);  // 固定圆心
    this->createFixPointConstraint(pt1);       // 固定直线起点（相切点）

    // 添加圆弧与直线相切约束
    this->addArcLineTangentConstraint(arcId, lineId);

    qDebug() << "EaSession: Created arc-line tangent constraint between arc" << arcId << "and line" << lineId;
    qDebug() << "EaSession: Points - centerPt:" << centerPt << "pt1:" << pt1 << "pt2:" << pt2;
}

void EaSession::createSymmConstraint()
{
    this->clear();
    
    // 创建四个点：两个对称点，对称轴的两个端点
    int pt1 = this->addPoint(50.0, 100.0);   // 对称点1（可拖动）
    int pt2 = this->addPoint(150.0, 100.0);  // 对称点2（自动对称）
    int pt3 = this->addPoint(100.0, 50.0);   // 对称轴端点1（固定）
    int pt4 = this->addPoint(100.0, 150.0);  // 对称轴端点2（固定）
    
    // 创建对称轴线段（独立的垂直线段）
    int line1 = this->addLine(pt3, pt4);     // 对称轴线段
    
    // 固定对称轴的两个端点，使对称轴保持稳定
    this->createFixPointConstraint(pt3);
    this->createFixPointConstraint(pt4);
    
    // 添加关于直线对称的约束
    this->addSymmetricLineConstraint(pt1, pt2, line1);
    
    qDebug() << "EaSession: Created symmetric line constraint with points" << pt1 << pt2 << "and line" << line1;
    qDebug() << "EaSession: Points" << pt3 << "and" << pt4 << "are fixed (symmetry axis), point" << pt1 << "can be dragged, point" << pt2 << "will maintain symmetry";
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

