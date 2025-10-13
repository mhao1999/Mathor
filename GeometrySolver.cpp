#include "GeometrySolver.h"
#include <QDebug>

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
                                        const QVariantMap& pointPositions,
                                        const QVariantList& constraints)
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
    QMap<int, int> pointToEntity; // 点ID到实体ID的映射
    QMap<int, int> pointToParamX; // 点ID到X参数索引的映射
    QMap<int, int> pointToParamY; // 点ID到Y参数索引的映射
    
    int paramIndex = 10;
    int entityIndex = 300;
    
    for (auto it = pointPositions.begin(); it != pointPositions.end(); ++it) {
        int pointId = it.key().toInt();
        QVariantMap pos = it.value().toMap();
        double x = pos["x"].toDouble();
        double y = pos["y"].toDouble();
        
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
        
        // 记录参数索引
        pointToParamX[pointId] = paramXIndex;
        pointToParamY[pointId] = paramYIndex;
        
        // 创建点实体
        m_sys.entity[m_sys.entities++] = Slvs_MakePoint2d(entityIndex, g, 200, paramXIndex, paramYIndex);
        pointToEntity[pointId] = entityIndex++;
        
        qDebug() << "GeometrySolver: Created point" << pointId << "at" << x << y 
                 << "with params" << paramXIndex << paramYIndex;
    }
    
    // 添加约束
    qDebug() << "GeometrySolver: Adding constraints, total constraints to add:" << constraints.size();
    for (const QVariant& constraintVar : constraints) {
        QVariantMap constraint = constraintVar.toMap();
        QString type = constraint["type"].toString();
        
        qDebug() << "GeometrySolver: Processing constraint type:" << type << "constraint:" << constraint;
        
        if (type == "distance") {
            int point1Id = constraint["point1"].toInt();
            int point2Id = constraint["point2"].toInt();
            double distance = constraint["distance"].toDouble();
            
            qDebug() << "GeometrySolver: Distance constraint - point1Id:" << point1Id 
                     << "point2Id:" << point2Id << "distance:" << distance;
            qDebug() << "GeometrySolver: pointToEntity contains point1Id:" << pointToEntity.contains(point1Id)
                     << "point2Id:" << pointToEntity.contains(point2Id);
            
            if (pointToEntity.contains(point1Id) && pointToEntity.contains(point2Id)) {
                int constraintId = m_sys.constraints + 1;
                m_sys.constraint[m_sys.constraints++] = Slvs_MakeConstraint(
                    constraintId, g,
                    SLVS_C_PT_PT_DISTANCE,
                    200,
                    distance,
                    pointToEntity[point1Id], pointToEntity[point2Id], 0, 0);
                
                qDebug() << "GeometrySolver: Added distance constraint" << constraintId 
                         << "between points" << point1Id << "and" << point2Id 
                         << "distance:" << distance << "entities:" << pointToEntity[point1Id] 
                         << pointToEntity[point2Id];
            } else {
                qWarning() << "GeometrySolver: Cannot add distance constraint - missing entities";
            }
        }
    }
    
    qDebug() << "GeometrySolver: Total constraints added:" << m_sys.constraints;
    
    // 设置拖拽约束 - 固定不被拖拽的点
    // 在SolveSpaceLib中，dragged数组包含应该被固定的参数
    // 被拖拽的点不应该在dragged数组中，这样它就可以自由移动
    int draggedCount = 0;
    for (auto it = pointPositions.begin(); it != pointPositions.end(); ++it) {
        int pointId = it.key().toInt();
        if (pointId != draggedPointId) {
            // 这个点不是被拖拽的点，应该被固定
            if (pointToParamX.contains(pointId) && pointToParamY.contains(pointId)) {
                if (draggedCount < 4) {  // SolveSpaceLib最多支持4个拖拽参数
                    m_sys.dragged[draggedCount++] = pointToParamX[pointId];
                    if (draggedCount < 4) {
                        m_sys.dragged[draggedCount++] = pointToParamY[pointId];
                    }
                }
            }
        }
    }
    
    // 填充剩余的拖拽参数为0
    for (int i = draggedCount; i < 4; i++) {
        m_sys.dragged[i] = 0;
    }
    
    qDebug() << "GeometrySolver: Setting drag constraints - fixed" << draggedCount << "parameters";
    qDebug() << "GeometrySolver: Dragged point" << draggedPointId << "is free to move";
    
    // 启用失败约束计算
    m_sys.calculateFaileds = 1;
    
    // 求解
    Slvs_Solve(&m_sys, g);
    
    // 处理结果
    m_dof = m_sys.dof;
    emit dofChanged();
    
    if (m_sys.result == SLVS_RESULT_OKAY) {
        // 保存求解后的坐标到成员变量（为了兼容现有接口）
        // 这里简化处理，只保存前两个点
        int pointCount = 0;
        for (int i = 0; i < m_sys.params && pointCount < 4; i++) {
            if (m_sys.param[i].group == g) {
                if (pointCount == 0) {
                    m_solvedX1 = m_sys.param[i].val;
                } else if (pointCount == 1) {
                    m_solvedY1 = m_sys.param[i].val;
                } else if (pointCount == 2) {
                    m_solvedX2 = m_sys.param[i].val;
                } else if (pointCount == 3) {
                    m_solvedY2 = m_sys.param[i].val;
                    break;
                }
                pointCount++;
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


