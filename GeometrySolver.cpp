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
        
        m_lastError = "求解成功";
        qDebug() << "求解成功!";
        qDebug() << "点1: (" << m_solvedX1 << ", " << m_solvedY1 << ")";
        qDebug() << "点2: (" << m_solvedX2 << ", " << m_solvedY2 << ")";
        qDebug() << "自由度: " << m_dof;
        
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

