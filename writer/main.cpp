#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "app/WriterController.h"
#include "ui/HandwritingCanvasItem.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("WriterQt"));
    QCoreApplication::setOrganizationName(QStringLiteral("Handwrite"));

    qmlRegisterType<HandwritingCanvasItem>("Writer", 1, 0, "HandwritingCanvasItem");

    WriterController writer;

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &warnings) {
        for (const QQmlError &err : warnings) qWarning() << "[qml]" << err.toString();
    });
    engine.rootContext()->setContextProperty("writerController", &writer);
    engine.rootContext()->setContextProperty("gcodeController", writer.gcode());
    engine.rootContext()->setContextProperty("grblConnection", writer.grbl());
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) return 1;
    return app.exec();
}
