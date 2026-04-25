#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "src/app/AppController.h"
#include "src/canvas/CanvasItem.h"
#include "src/core/EditorStore.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qInfo() << "[boot] HandwriteQt starting";
    qInfo() << "[boot] Qt platform:" << QGuiApplication::platformName();
    qInfo() << "[boot] WAYLAND_DISPLAY=" << qEnvironmentVariable("WAYLAND_DISPLAY")
            << "DISPLAY=" << qEnvironmentVariable("DISPLAY");

    qmlRegisterType<CanvasItem>("Handwrite", 1, 0, "CanvasItem");

    EditorStore store;
    AppController controller(&store);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &warnings) {
        for (const QQmlError &err : warnings) {
            qWarning() << "[qml-warning]" << err.toString();
        }
    });
    engine.rootContext()->setContextProperty("editorStoreModel", &store);
    engine.rootContext()->setContextProperty("appController", &controller);
    qInfo() << "[boot] context properties set: editorStoreModel/appController";

    engine.load(QUrl("qrc:/qml/Main.qml"));
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "[boot] QML root object load failed";
        return 1;
    }
    qInfo() << "[boot] QML loaded, entering event loop";
    return app.exec();
}
