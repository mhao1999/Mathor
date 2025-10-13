#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "GeometrySolver.h"
#include "main/eadrawingarea.h"
#include "main/easession.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    // 注册自定义类型到QML
    qmlRegisterType<GeometrySolver>("Mathor.Solver", 1, 0, "GeometrySolver");
    qmlRegisterType<EaDrawingArea>("Mathor.Drawing", 1, 0, "DrawingArea");
    // qmlRegisterType<EaSession>("Mathor.Session", 1, 0, "EaSession");

    // 创建一个GeometrySolver实例用于测试
    GeometrySolver solver;
    qDebug() << "===============================================";
    qDebug() << "测试 SolveSpaceLib 集成";
    qDebug() << "===============================================";
    qDebug() << "求解问题: 两点距离约束";
    qDebug() << "初始点1: (10, 20)";
    qDebug() << "初始点2: (50, 60)";
    qDebug() << "目标距离: 100.0";
    qDebug() << "-----------------------------------------------";
    
    // 测试: 两点距离约束为100单位
    bool success = solver.solveSimple2DDistance(10, 20, 50, 60, 100.0);
    
    if (success) {
        QVariantMap result = solver.getSolvedPoints();
        qDebug() << "-----------------------------------------------";
        qDebug() << "求解后点1: (" << result["x1"].toDouble() << ", " 
                 << result["y1"].toDouble() << ")";
        qDebug() << "求解后点2: (" << result["x2"].toDouble() << ", " 
                 << result["y2"].toDouble() << ")";
        qDebug() << "自由度(DOF):" << solver.dof();
        qDebug() << "===============================================";
        qDebug() << "✓ SolveSpaceLib 集成成功!";
        qDebug() << "===============================================";
    } else {
        qDebug() << "错误:" << solver.lastError();
        qDebug() << "===============================================";
    }

    QQmlApplicationEngine engine;
    
    // 获取EaSession单例实例
    EaSession* session = EaSession::getInstance();
    
    // 将solver和session实例作为上下文属性暴露给QML
    engine.rootContext()->setContextProperty("globalSolver", &solver);
    engine.rootContext()->setContextProperty("globalSession", session);
    
    const QUrl url(QStringLiteral("qrc:/DrawingAreaDemo.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
