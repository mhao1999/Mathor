#include "easession.h"
#include <QDebug>
#include <algorithm>

EaSession *EaSession::instance = nullptr;

EaSession::EaSession() : QObject()
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
    m_selectedPoints.clear();
    m_selectedLines.clear();
    m_nextPointId = 1;
    m_nextLineId = 1;
    
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
