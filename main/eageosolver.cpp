#include "eageosolver.h"
#include "easession.h"
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
                                        const std::map<std::string, std::map<std::string, std::any>>& pointPositions,
                                        const std::vector<Constraint>& constraints)
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
    std::map<int, int> pointToParamX; // 点ID到X参数索引的映射
    std::map<int, int> pointToParamY; // 点ID到Y参数索引的映射
    
    int paramIndex = 10;
    int entityIndex = 300;
    
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
    
    // 添加点1的固定约束，确保点1不会移动
    if (pointToEntity.find(1) != pointToEntity.end()) {
        // 获取点1的初始坐标
        const auto& point1Pos = pointPositions.at("1");
        double x1 = std::any_cast<double>(point1Pos.at("x"));
        double y1 = std::any_cast<double>(point1Pos.at("y"));
        
        // 添加点1的固定约束 - 使用拖拽约束
        int constraintId = m_sys.constraints + 1;
        m_sys.constraint[m_sys.constraints++] = Slvs_MakeConstraint(
            constraintId, g,
            SLVS_C_WHERE_DRAGGED,
            200,
            0.0,
            pointToEntity[1], 0, 0, 0);
        
        qDebug() << "GeometrySolver: Added point1 fixed constraint" << constraintId
                 << "for entity" << pointToEntity[1] << "at position" << x1 << y1;
    }
    
    qDebug() << "GeometrySolver: Total constraints added:" << m_sys.constraints;
    
    // 不使用dragged数组，而是通过约束来固定点1
    // 清空dragged数组
    for (int i = 0; i < 4; i++) {
        m_sys.dragged[i] = 0;
    }
    
    // 使用dragged数组来指定被拖拽的参数（点2）
    // dragged数组中的参数是会被拖拽的参数，不是被固定的参数
    if (pointToParamX.find(draggedPointId) != pointToParamX.end() && pointToParamY.find(draggedPointId) != pointToParamY.end()) {
        m_sys.dragged[0] = pointToParamX[draggedPointId];  // 拖拽点2的X参数
        m_sys.dragged[1] = pointToParamY[draggedPointId];  // 拖拽点2的Y参数
        m_sys.dragged[2] = 0;  // 未使用
        m_sys.dragged[3] = 0;  // 未使用
        
        qDebug() << "GeometrySolver: Dragged point" << draggedPointId << "parameters:" 
                 << pointToParamX[draggedPointId] << pointToParamY[draggedPointId];
    }
    
    qDebug() << "GeometrySolver: Using dragged array to specify dragged parameters";
    qDebug() << "GeometrySolver: Point1 should be fixed, point2 should move";
    
    // 输出每个点的参数ID
    for (auto it = pointToParamX.begin(); it != pointToParamX.end(); ++it) {
        int pointId = it->first;
        qDebug() << "GeometrySolver: Point" << pointId << "X param:" << it->second 
                 << "Y param:" << pointToParamY[pointId];
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
                for (auto it = pointToParamX.begin(); it != pointToParamX.end(); ++it) {
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
                for (auto it = pointToParamY.begin(); it != pointToParamY.end(); ++it) {
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


