#include "GcodeController.h"

#include "GcodeGenerator.h"
#include "PathBuilder.h"
#include "app/WriterController.h"

#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QClipboard>

GcodeController::GcodeController(WriterController *writer, QObject *parent)
    : QObject(parent), m_writer(writer) {
    if (m_writer) {
        connect(m_writer, &WriterController::layoutInvalidated, this, &GcodeController::onLayoutInvalidated);
        connect(m_writer->settings(), &AppSettings::anyChanged, this, &GcodeController::onLayoutInvalidated);
    }
    regenerate();
}

void GcodeController::onLayoutInvalidated() {
    setGcodeStale(true);
    regenerate();
}

void GcodeController::regenerate() {
    if (!m_writer) {
        setGeneratedGcode(QStringLiteral("; No document\n"));
        return;
    }
    const PathBuildResult path = PathBuilder::buildFromController(m_writer);
    const QString gcode = GcodeGenerator::generate(path, m_writer->settings());
    setGeneratedGcode(gcode);
    setGcodeStale(false);
}

void GcodeController::copyToClipboard() {
    if (QClipboard *cb = QGuiApplication::clipboard())
        cb->setText(m_generatedGcode);
}

void GcodeController::saveGcodeFile() {
    const QString path = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Save G-code"),
        QString(),
        QStringLiteral("G-code (*.gcode);;All files (*)")
    );
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return;
    file.write(m_generatedGcode.toUtf8());
    file.close();
}

void GcodeController::setGeneratedGcode(const QString &gcode) {
    if (m_generatedGcode == gcode) return;
    m_generatedGcode = gcode;
    emit generatedGcodeChanged();
}

void GcodeController::setGcodeStale(bool stale) {
    if (m_gcodeStale == stale) return;
    m_gcodeStale = stale;
    emit gcodeStaleChanged();
}
