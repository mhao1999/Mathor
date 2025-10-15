#include "eageosolver.h"
#include "easession.h"
#include <QDebug>
#include <set>

GeometrySolver::GeometrySolver(QObject *parent)
    : QObject(parent)
    , m_dof(0)
    , m_solvedX1(0), m_solvedY1(0)
    , m_solvedX2(0), m_solvedY2(0)
{
    initSystem();
}

GeometrySolver::~GeometrySolver()
{
    clearSystem();
}

void GeometrySolver::initSystem()
{
    memset(&m_sys, 0, sizeof(m_sys));
    
    // 分配内存
    m_sys.param = (Slvs_Param*)malloc(50 * sizeof(Slvs_Param));
    m_sys.entity = (Slvs_Entity*)malloc(50 * sizeof(Slvs_Entity));
    m_sys.constraint = (Slvs_Constraint*)malloc(50 * sizeof(Slvs_Constraint));
    m_sys.failed = (Slvs_hConstraint*)malloc(50 * sizeof(Slvs_hConstraint));
    m_sys.faileds = 50;
}

void GeometrySolver::clearSystem()
{
    if (m_sys.param) free(m_sys.param);
    if (m_sys.entity) free(m_sys.entity);
    if (m_sys.constraint) free(m_sys.constraint);
    if (m_sys.failed) free(m_sys.failed);
    
    memset(&m_sys, 0, sizeof(m_sys));
}

bool GeometrySolver::solveSimple2DDistance(double x1, double y1, 
                                            double x2, double y2, 
                                            double targetDistance)
{
    // 重置系统
    m_sys.params = 0;
    m_sys.entities = 0;
    m_sys.constraints = 0;
    
    Slvs_hGroup g = 1;
    double qw, qx, qy, qz;
    
    // 创建工作平面
    // 原点在(0, 0, 0)
    m_sys.param[m_sys.params++] = Slvs_MakeParam(1, g, 0.0);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(2, g, 0.0);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(3, g, 0.0);
    m_sys.entity[m_sys.entities++] = Slvs_MakePoint3d(101, g, 1, 2, 3);
    
    // 工作平面平行于xy平面
    Slvs_MakeQuaternion(1, 0, 0, 0, 1, 0, &qw, &qx, &qy, &qz);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(4, g, qw);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(5, g, qx);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(6, g, qy);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(7, g, qz);
    m_sys.entity[m_sys.entities++] = Slvs_MakeNormal3d(102, g, 4, 5, 6, 7);
    
    m_sys.entity[m_sys.entities++] = Slvs_MakeWorkplane(200, g, 101, 102);
    
    // 创建第二个组进行求解
    g = 2;
    
    // 第一个点
    m_sys.param[m_sys.params++] = Slvs_MakeParam(11, g, x1);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(12, g, y1);
    m_sys.entity[m_sys.entities++] = Slvs_MakePoint2d(301, g, 200, 11, 12);
    
    // 第二个点
    m_sys.param[m_sys.params++] = Slvs_MakeParam(13, g, x2);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(14, g, y2);
    m_sys.entity[m_sys.entities++] = Slvs_MakePoint2d(302, g, 200, 13, 14);
    
    // 添加距离约束
    m_sys.constraint[m_sys.constraints++] = Slvs_MakeConstraint(
        1, g,
        SLVS_C_PT_PT_DISTANCE,
        200,
        targetDistance,
        301, 302, 0, 0);
    
    // 固定第一个点（拖拽约束）
    m_sys.dragged[0] = 11;
    m_sys.dragged[1] = 12;
    m_sys.dragged[2] = 0;
    m_sys.dragged[3] = 0;
    
    // 启用失败约束计算
    m_sys.calculateFaileds = 1;
    
    // 求解
    Slvs_Solve(&m_sys, g);
    
    // 处理结果
    m_dof = m_sys.dof;
    emit dofChanged();
    
    if (m_sys.result == SLVS_RESULT_OKAY) {
        // 保存求解后的坐标
        m_solvedX1 = m_sys.param[7].val;   // param[11] 的索引是 7
        m_solvedY1 = m_sys.param[8].val;   // param[12] 的索引是 8
        m_solvedX2 = m_sys.param[9].val;   // param[13] 的索引是 9
        m_solvedY2 = m_sys.param[10].val;  // param[14] 的索引是 10
        
        m_lastError = "solve success";
        qDebug() << "solve success!";
        qDebug() << "pt 1: (" << m_solvedX1 << ", " << m_solvedY1 << ")";
        qDebug() << "pt 2: (" << m_solvedX2 << ", " << m_solvedY2 << ")";
        qDebug() << "dof: " << m_dof;
        
        emit lastErrorChanged();
        emit solvingFinished(true);
        return true;
    } else {
        m_lastError = getResultMessage(m_sys.result);
        
        if (m_sys.faileds > 0) {
            m_lastError += " - 问题约束: ";
            for (int i = 0; i < m_sys.faileds; i++) {
                m_lastError += QString::number(m_sys.failed[i]) + " ";
            }
        }
        
        qWarning() << "求解失败:" << m_lastError;
        emit lastErrorChanged();
        emit solvingFinished(false);
        return false;
    }
}

bool GeometrySolver::solveDragConstraint(int draggedPointId, double newX, double newY,
                                        const std::map<std::string, std::map<std::string, std::any>>& pointPositions,
                                        const std::vector<Constraint>& constraints,
                                        const std::map<std::string, std::map<std::string, std::any>>& lineInfo)
{
    qDebug() << "GeometrySolver: solveDragConstraint called for point" << draggedPointId 
             << "to position" << newX << newY;
    
    // 重置系统
    m_sys.params = 0;
    m_sys.entities = 0;
    m_sys.constraints = 0;
    
    Slvs_hGroup g = 1;
    double qw, qx, qy, qz;
    
    // 创建工作平面
    m_sys.param[m_sys.params++] = Slvs_MakeParam(1, g, 0.0);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(2, g, 0.0);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(3, g, 0.0);
    m_sys.entity[m_sys.entities++] = Slvs_MakePoint3d(101, g, 1, 2, 3);
    
    // 工作平面平行于xy平面
    Slvs_MakeQuaternion(1, 0, 0, 0, 1, 0, &qw, &qx, &qy, &qz);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(4, g, qw);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(5, g, qx);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(6, g, qy);
    m_sys.param[m_sys.params++] = Slvs_MakeParam(7, g, qz);
    m_sys.entity[m_sys.entities++] = Slvs_MakeNormal3d(102, g, 4, 5, 6, 7);
    
    m_sys.entity[m_sys.entities++] = Slvs_MakeWorkplane(200, g, 101, 102);
    
    // 创建第二个组进行求解
    g = 2;
    
    // 创建所有点，并记录参数索引
    std::map<int, int> pointToEntity; // 点ID到实体ID的映射
    std::map<int, int> lineToEntity;  // 线段ID到实体ID的映射
    std::map<int, int> centerToCircleEntity; // 圆心点ID到圆实体ID的映射
    // 清空之前的映射
    m_pointToParamX.clear();
    m_pointToParamY.clear();
    
    int paramIndex = 10;  // 从10开始，避免与工作平面参数ID冲突
    int entityIndex = 300;
    
    // 创建所有点
    for (const auto& it : pointPositions) {
        int pointId = std::stoi(it.first);
        const auto& pos = it.second;
        double x = std::any_cast<double>(pos.at("x"));
        double y = std::any_cast<double>(pos.at("y"));
        
        // 如果是被拖拽的点，使用新位置
        if (pointId == draggedPointId) {
            x = newX;
            y = newY;
            qDebug() << "GeometrySolver: Using new position for dragged point" << pointId << ":" << x << y;
        }
        
        // 创建参数并记录索引
        int paramXIndex = paramIndex++;
        m_sys.param[m_sys.params++] = Slvs_MakeParam(paramXIndex, g, x);
        int paramYIndex = paramIndex++;
        m_sys.param[m_sys.params++] = Slvs_MakeParam(paramYIndex, g, y);
        
        // 记录参数索引到成员变量
        m_pointToParamX[pointId] = paramXIndex;
        m_pointToParamY[pointId] = paramYIndex;
        
        // 创建点实体
        m_sys.entity[m_sys.entities++] = Slvs_MakePoint2d(entityIndex, g, 200, paramXIndex, paramYIndex);
        pointToEntity[pointId] = entityIndex++;
        
        qDebug() << "GeometrySolver: Created point" << pointId << "at" << x << y 
                 << "with params" << paramXIndex << paramYIndex << "in group" << g;
    }
    
    // 创建线段实体
    for (const auto& it : lineInfo) {
        int lineId = std::stoi(it.first);
        const auto& lineData = it.second;
        int startPointId = std::any_cast<int>(lineData.at("startPoint"));
        int endPointId = std::any_cast<int>(lineData.at("endPoint"));
        
        if (pointToEntity.find(startPointId) != pointToEntity.end() && 
            pointToEntity.find(endPointId) != pointToEntity.end()) {
            
            // 创建线段实体
            m_sys.entity[m_sys.entities++] = Slvs_MakeLineSegment(entityIndex, g, 200, 
                                                                  pointToEntity[startPointId], 
                                                                  pointToEntity[endPointId]);
            lineToEntity[lineId] = entityIndex++;
            
            qDebug() << "GeometrySolver: Created line" << lineId << "from point" << startPointId 
                     << "to point" << endPointId << "with entity" << lineToEntity[lineId];
        } else {
            qWarning() << "GeometrySolver: Cannot create line" << lineId << "- missing points" << startPointId << endPointId;
        }
    }
    
    // 添加约束（包括创建圆实体）
    qDebug() << "GeometrySolver: Adding constraints, total constraints to add:" << constraints.size();
    for (const Constraint& constraint : constraints) {
        std::string type = constraint.type;
        
        qDebug() << "GeometrySolver: Processing constraint type:" << type.c_str() << "constraint id:" << constraint.id;
        
        if (type == "distance") {
            int point1Id = std::any_cast<int>(constraint.data.at("point1"));
            int point2Id = std::any_cast<int>(constraint.data.at("point2"));
            double distance = std::any_cast<double>(constraint.data.at("distance"));
            
            qDebug() << "GeometrySolver: Distance constraint - point1Id:" << point1Id 
                     << "point2Id:" << point2Id << "distance:" << distance;
            qDebug() << "GeometrySolver: pointToEntity contains point1Id:" << (pointToEntity.find(point1Id) != pointToEntity.end())
                     << "point2Id:" << (pointToEntity.find(point2Id) != pointToEntity.end());
            
            if (pointToEntity.find(point1Id) != pointToEntity.end() && pointToEntity.find(point2Id) != pointToEntity.end()) {
                int constraintId = m_sys.constraints + 1;
                Slvs_Constraint constraint = Slvs_MakeConstraint(
                    constraintId, g,
                    SLVS_C_PT_PT_DISTANCE,
                    200,
                    distance,
                    pointToEntity[point1Id], pointToEntity[point2Id], 0, 0);
                constraint.entityC = 0;
                constraint.entityD = 0;
                m_sys.constraint[m_sys.constraints++] = constraint;
                
                qDebug() << "GeometrySolver: Added distance constraint" << constraintId 
                         << "between points" << point1Id << "and" << point2Id 
                         << "distance:" << distance << "entities:" << pointToEntity[point1Id] 
                         << pointToEntity[point2Id];
            } else {
                qWarning() << "GeometrySolver: Cannot add distance constraint - missing entities";
            }
        }
        else if (type == "fix_point") {
            int pointId = std::any_cast<int>(constraint.data.at("point"));
            
            if (pointToEntity.find(pointId) != pointToEntity.end()) {
                // 添加固定点约束 - 使用WHERE_DRAGGED约束来固定点
                int constraintId = m_sys.constraints + 1;
                Slvs_Constraint constraint = Slvs_MakeConstraint(
                    constraintId, g,
                    SLVS_C_WHERE_DRAGGED,
                    200,
                    0.0,
                    pointToEntity[pointId], 0, 0, 0);
                constraint.entityC = 0;
                constraint.entityD = 0;
                m_sys.constraint[m_sys.constraints++] = constraint;
                
                qDebug() << "GeometrySolver: Added fix point constraint" << constraintId
                         << "for point" << pointId << "entity" << pointToEntity[pointId];
            } else {
                qWarning() << "GeometrySolver: Cannot add fix point constraint - point" << pointId << "not found";
            }
        }
        else if (type == "drag_point") {
            // 拖拽约束现在通过dragged数组处理，不需要添加SLVS_C_WHERE_DRAGGED约束
            qDebug() << "GeometrySolver: Skipping drag_point constraint - handled by dragged array";
        }
        else if (type == "parallel") {
            int line1Id = std::any_cast<int>(constraint.data.at("line1"));
            int line2Id = std::any_cast<int>(constraint.data.at("line2"));
            
            if (lineToEntity.find(line1Id) != lineToEntity.end() && 
                lineToEntity.find(line2Id) != lineToEntity.end()) {
                
                // 添加平行约束
                int constraintId = m_sys.constraints + 1;
                Slvs_Constraint constraint = Slvs_MakeConstraint(
                    constraintId, g,
                    SLVS_C_PARALLEL,
                    200,
                    0.0,
                    0, 0,
                    lineToEntity[line1Id], lineToEntity[line2Id]);
                constraint.entityC = 0;
                constraint.entityD = 0;
                m_sys.constraint[m_sys.constraints++] = constraint;
                
                qDebug() << "GeometrySolver: Added parallel constraint" << constraintId
                         << "between lines" << line1Id << "and" << line2Id 
                         << "entities" << lineToEntity[line1Id] << lineToEntity[line2Id];
            } else {
                qWarning() << "GeometrySolver: Cannot add parallel constraint - missing line entities" << line1Id << line2Id;
            }
        }
        else if (type == "perpendicular") {
            int line1Id = std::any_cast<int>(constraint.data.at("line1"));
            int line2Id = std::any_cast<int>(constraint.data.at("line2"));
            
            if (lineToEntity.find(line1Id) != lineToEntity.end() && 
                lineToEntity.find(line2Id) != lineToEntity.end()) {
                
                // 添加垂直约束
                int constraintId = m_sys.constraints + 1;
                Slvs_Constraint constraint = Slvs_MakeConstraint(
                    constraintId, g,
                    SLVS_C_PERPENDICULAR,
                    200,
                    0.0,
                    0, 0,
                    lineToEntity[line1Id], lineToEntity[line2Id]);
                constraint.entityC = 0;
                constraint.entityD = 0;
                m_sys.constraint[m_sys.constraints++] = constraint;
                
                qDebug() << "GeometrySolver: Added perpendicular constraint" << constraintId
                         << "between lines" << line1Id << "and" << line2Id 
                         << "entities" << lineToEntity[line1Id] << lineToEntity[line2Id];
            } else {
                qWarning() << "GeometrySolver: Cannot add perpendicular constraint - missing line entities" << line1Id << line2Id;
            }
        }
        else if (type == "horizontal") {
            int lineId = std::any_cast<int>(constraint.data.at("line"));
            
            if (lineToEntity.find(lineId) != lineToEntity.end()) {
                
                // 添加水平约束
                int constraintId = m_sys.constraints + 1;
                Slvs_Constraint constraint = Slvs_MakeConstraint(
                    constraintId, g,
                    SLVS_C_HORIZONTAL,
                    200,
                    0.0,
                    0, 0, lineToEntity[lineId], 0);
                constraint.entityC = 0;
                constraint.entityD = 0;
                m_sys.constraint[m_sys.constraints++] = constraint;
                
                qDebug() << "GeometrySolver: Added horizontal constraint" << constraintId
                         << "for line" << lineId << "entity" << lineToEntity[lineId];
            } else {
                qWarning() << "GeometrySolver: Cannot add horizontal constraint - missing line entity" << lineId;
            }
        }
        else if (type == "vertical") {
            int lineId = std::any_cast<int>(constraint.data.at("line"));

            if (lineToEntity.find(lineId) != lineToEntity.end()) {

                // 添加水平约束
                int constraintId = m_sys.constraints + 1;
                Slvs_Constraint constraint = Slvs_MakeConstraint(
                    constraintId, g,
                    SLVS_C_VERTICAL,
                    200,
                    0.0,
                    0, 0, lineToEntity[lineId], 0);
                constraint.entityC = 0;
                constraint.entityD = 0;
                m_sys.constraint[m_sys.constraints++] = constraint;

                qDebug() << "GeometrySolver: Added vertical constraint" << constraintId
                         << "for line" << lineId << "entity" << lineToEntity[lineId];
            } else {
                qWarning() << "GeometrySolver: Cannot add vertical constraint - missing line entity" << lineId;
            }
        }
        else if (type == "pt_on_line") {
            int pointId = std::any_cast<int>(constraint.data.at("point"));
            int lineId = std::any_cast<int>(constraint.data.at("line"));
            
            if (pointToEntity.find(pointId) != pointToEntity.end() && 
                lineToEntity.find(lineId) != lineToEntity.end()) {
                
                // 添加点在线上约束
                int constraintId = m_sys.constraints + 1;
                Slvs_Constraint constraint = Slvs_MakeConstraint(
                    constraintId, g,
                    SLVS_C_PT_ON_LINE,
                    200,
                    0.0,
                    pointToEntity[pointId], 0, lineToEntity[lineId], 0);
                constraint.entityC = 0;
                constraint.entityD = 0;
                m_sys.constraint[m_sys.constraints++] = constraint;
                
                qDebug() << "GeometrySolver: Added point on line constraint" << constraintId
                         << "for point" << pointId << "on line" << lineId 
                         << "entities" << pointToEntity[pointId] << lineToEntity[lineId];
            } else {
                qWarning() << "GeometrySolver: Cannot add point on line constraint - missing point or line entities" << pointId << lineId;
            }
        }
        else if (type == "pt_on_circle") {
            int pointId = std::any_cast<int>(constraint.data.at("point"));
            int centerPointId = std::any_cast<int>(constraint.data.at("center"));
            double radius = std::any_cast<double>(constraint.data.at("radius"));
            
            // 首先检查是否需要创建圆实体
            if (centerToCircleEntity.find(centerPointId) == centerToCircleEntity.end() &&
                pointToEntity.find(centerPointId) != pointToEntity.end()) {
                
                // 创建半径距离实体
                int radiusParamIndex = paramIndex++;
                m_sys.param[m_sys.params++] = Slvs_MakeParam(radiusParamIndex, g, radius);
                
                int radiusEntityIndex = entityIndex++;
                m_sys.entity[m_sys.entities++] = Slvs_MakeDistance(radiusEntityIndex, g, 200, radiusParamIndex);
                
                // 创建圆实体
                int circleEntityIndex = entityIndex++;
                m_sys.entity[m_sys.entities++] = Slvs_MakeCircle(circleEntityIndex, g, 200,
                                                                 pointToEntity[centerPointId], 102, radiusEntityIndex);
                
                // 使用圆心点ID作为键
                centerToCircleEntity[centerPointId] = circleEntityIndex;
                
                // 添加直径约束来固定圆的半径
                int diameterConstraintId = m_sys.constraints + 1;
                Slvs_Constraint diameterConstraint = Slvs_MakeConstraint(
                    diameterConstraintId, g,
                    SLVS_C_DIAMETER,
                    200,
                    radius * 2.0,  // 直径 = 半径 * 2
                    0, 0, circleEntityIndex, 0);
                diameterConstraint.entityC = 0;
                diameterConstraint.entityD = 0;
                m_sys.constraint[m_sys.constraints++] = diameterConstraint;
                
                qDebug() << "GeometrySolver: Created circle with center point" << centerPointId 
                         << "radius" << radius << "entity" << circleEntityIndex;
                qDebug() << "GeometrySolver: Added diameter constraint" << diameterConstraintId 
                         << "for circle" << circleEntityIndex << "diameter" << (radius * 2.0);
            } else if (centerToCircleEntity.find(centerPointId) != centerToCircleEntity.end()) {
                qDebug() << "GeometrySolver: Circle already exists for center point" << centerPointId;
            } else {
                qWarning() << "GeometrySolver: Cannot create circle - missing center point" << centerPointId;
            }
            
            // 然后添加点在圆上约束
            if (pointToEntity.find(pointId) != pointToEntity.end() && 
                centerToCircleEntity.find(centerPointId) != centerToCircleEntity.end()) {
                
                // 添加点在圆上约束
                int constraintId = m_sys.constraints + 1;
                Slvs_Constraint constraint = Slvs_MakeConstraint(
                    constraintId, g,
                    SLVS_C_PT_ON_CIRCLE,
                    200,
                    0.0,
                    pointToEntity[pointId], 0, centerToCircleEntity[centerPointId], 0);
                constraint.entityC = 0;
                constraint.entityD = 0;
                m_sys.constraint[m_sys.constraints++] = constraint;
                
                qDebug() << "GeometrySolver: Added point on circle constraint" << constraintId
                         << "for point" << pointId << "on circle with center" << centerPointId 
                         << "entities" << pointToEntity[pointId] << centerToCircleEntity[centerPointId];
            } else {
                qWarning() << "GeometrySolver: Cannot add point on circle constraint - missing point or circle entities" << pointId << centerPointId;
            }
        }
    }
    
    qDebug() << "GeometrySolver: Total constraints added:" << m_sys.constraints;
    
    // 不使用dragged数组，直接进行约束求解
    // 清空dragged数组
    for (int i = 0; i < 4; i++) {
        m_sys.dragged[i] = 0;
    }
    
    qDebug() << "GeometrySolver: Not using dragged array - relying on constraints only";
    
    // 输出每个点的参数ID
    for (auto it = m_pointToParamX.begin(); it != m_pointToParamX.end(); ++it) {
        int pointId = it->first;
        qDebug() << "GeometrySolver: Point" << pointId << "X param:" << it->second 
                 << "Y param:" << m_pointToParamY[pointId];
    }
    
    // 启用失败约束计算
    m_sys.calculateFaileds = 1;
    
    // 求解
    Slvs_Solve(&m_sys, g);
    
    // 处理结果
    m_dof = m_sys.dof;
    emit dofChanged();
    
    if (m_sys.result == SLVS_RESULT_OKAY) {
        // 保存求解后的坐标到成员变量（为了兼容现有接口）
        // 按照点ID来分配坐标，而不是按照参数数组顺序
        for (int i = 0; i < m_sys.params; i++) {
            if (m_sys.param[i].group == g) {
                // 查找这个参数对应的点ID
                for (auto it = m_pointToParamX.begin(); it != m_pointToParamX.end(); ++it) {
                    int pointId = it->first;
                    if (it->second == m_sys.param[i].h) {
                        // 这是点ID的X坐标
                        if (pointId == 1) {
                            m_solvedX1 = m_sys.param[i].val;
                        } else if (pointId == 2) {
                            m_solvedX2 = m_sys.param[i].val;
                        }
                        break;
                    }
                }
                for (auto it = m_pointToParamY.begin(); it != m_pointToParamY.end(); ++it) {
                    int pointId = it->first;
                    if (it->second == m_sys.param[i].h) {
                        // 这是点ID的Y坐标
                        if (pointId == 1) {
                            m_solvedY1 = m_sys.param[i].val;
                        } else if (pointId == 2) {
                            m_solvedY2 = m_sys.param[i].val;
                        }
                        break;
                    }
                }
            }
        }
        
        m_lastError = "拖拽约束求解成功";
        qDebug() << "GeometrySolver: drag constraint solve successfully!";
        qDebug() << "GeometrySolver: pt 1 after solve: (" << m_solvedX1 << ", " << m_solvedY1 << ")";
        qDebug() << "GeometrySolver: pt 2 after solve: (" << m_solvedX2 << ", " << m_solvedY2 << ")";
        qDebug() << "GeometrySolver: dof: " << m_dof;
        
        emit lastErrorChanged();
        emit solvingFinished(true);
        return true;
    } else {
        m_lastError = getResultMessage(m_sys.result);
        
        if (m_sys.faileds > 0) {
            m_lastError += " - 问题约束: ";
            for (int i = 0; i < m_sys.faileds; i++) {
                m_lastError += QString::number(m_sys.failed[i]) + " ";
            }
        }
        
        qWarning() << "GeometrySolver: 拖拽约束求解失败:" << m_lastError;
        emit lastErrorChanged();
        emit solvingFinished(false);
        return false;
    }
}

QVariantMap GeometrySolver::getSolvedPoints()
{
    QVariantMap result;
    result["x1"] = m_solvedX1;
    result["y1"] = m_solvedY1;
    result["x2"] = m_solvedX2;
    result["y2"] = m_solvedY2;
    return result;
}

QVariantMap GeometrySolver::getSolvedPoints(const std::map<std::string, std::map<std::string, std::any>>& pointPositions)
{
    QVariantMap result;
    
    // 使用保存的参数映射来获取所有点的求解结果
    for (const auto& pointIt : pointPositions) {
        std::string pointIdStr = pointIt.first;
        int pointId = std::stoi(pointIdStr);
        
        // 查找该点的X和Y参数
        if (m_pointToParamX.find(pointId) != m_pointToParamX.end() && 
            m_pointToParamY.find(pointId) != m_pointToParamY.end()) {
            
            int paramXIndex = m_pointToParamX[pointId];
            int paramYIndex = m_pointToParamY[pointId];
            
            // 从参数数组中获取求解后的坐标
            for (int i = 0; i < m_sys.params; i++) {
                if (m_sys.param[i].h == paramXIndex) {
                    result[QString("x%1").arg(pointId)] = m_sys.param[i].val;
                } else if (m_sys.param[i].h == paramYIndex) {
                    result[QString("y%1").arg(pointId)] = m_sys.param[i].val;
                }
            }
        }
    }
    
    return result;
}

QString GeometrySolver::getResultMessage(int result)
{
    switch (result) {
        case SLVS_RESULT_OKAY:
            return "成功";
        case SLVS_RESULT_INCONSISTENT:
            return "约束系统不一致（过约束或矛盾约束）";
        case SLVS_RESULT_DIDNT_CONVERGE:
            return "求解器未收敛";
        case SLVS_RESULT_TOO_MANY_UNKNOWNS:
            return "未知数过多";
        default:
            return "未知错误";
    }
}


