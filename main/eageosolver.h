#ifndef EAGEOSOLVER_H
#define EAGEOSOLVER_H

#include <QObject>
#include <QVariantMap>
#include <map>
#include <string>
#include <any>
#include <vector>
#include <slvs.h>

// 前向声明
struct Constraint;

/**
 * @brief GeometrySolver类 - SolveSpaceLib的Qt封装
 * 
 * 这个类提供了一个面向Qt/QML的接口来使用SolveSpaceLib几何求解器
 */
class GeometrySolver : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int dof READ dof NOTIFY dofChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit GeometrySolver(QObject *parent = nullptr);
    ~GeometrySolver();

    // 属性访问器
    int dof() const { return m_dof; }
    QString lastError() const { return m_lastError; }

    // 简单的2D两点距离约束示例
    Q_INVOKABLE bool solveSimple2DDistance(double x1, double y1, 
                                            double x2, double y2, 
                                            double targetDistance);

    // 拖拽约束求解 - 拖拽一个点，其他点根据约束调整
    Q_INVOKABLE bool solveDragConstraint(int draggedPointId, double newX, double newY,
                                        const std::map<std::string, std::map<std::string, std::any>>& pointPositions,
                                        const std::vector<Constraint>& constraints,
                                        const std::map<std::string, std::map<std::string, std::any>>& lineInfo = {});

    // 获取求解后的点坐标
    Q_INVOKABLE QVariantMap getSolvedPoints();
    Q_INVOKABLE QVariantMap getSolvedPoints(const std::map<std::string, std::map<std::string, std::any>>& pointPositions);

signals:
    void dofChanged();
    void lastErrorChanged();
    void solvingFinished(bool success);

private:
    void initSystem();
    void clearSystem();
    QString getResultMessage(int result);

    Slvs_System m_sys;
    int m_dof;
    QString m_lastError;
    
    // 用于存储求解后的结果
    double m_solvedX1, m_solvedY1;
    double m_solvedX2, m_solvedY2;
    
    // 用于存储参数映射，支持动态数量的点
    std::map<int, int> m_pointToParamX; // 点ID到X参数索引的映射
    std::map<int, int> m_pointToParamY; // 点ID到Y参数索引的映射
};

#endif // EAGEOSOLVER_H


