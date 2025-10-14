#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "main/eageosolver.h"
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

    QQmlApplicationEngine engine;
    
    // 获取EaSession单例实例
    EaSession* session = EaSession::getInstance();
    
    // 设置GeometrySolver引用到EaSession
    session->setGeometrySolver(&solver);
    
    // 将solver和session实例作为上下文属性暴露给QML
    engine.rootContext()->setContextProperty("globalSolver", &solver);
    engine.rootContext()->setContextProperty("globalSession", session);
    
    const QUrl url(QStringLiteral("qrc:/main.qml"));
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
