#ifndef GEOMETRYSOLVER_H
#define GEOMETRYSOLVER_H

#include <QObject>
#include <QVariantMap>
#include <slvs.h>

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

    // 获取求解后的点坐标
    Q_INVOKABLE QVariantMap getSolvedPoints();

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
};

#endif // GEOMETRYSOLVER_H

