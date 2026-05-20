#include "GcodeController.h"

#include "GcodeGenerator.h"
#include "PathBuilder.h"
#include "app/WriterController.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QClipboard>

GcodeController::GcodeController(WriterController *writer, QObject *parent)
    : QObject(parent), m_writer(writer) {
    if (m_writer) {
        connect(m_writer, &WriterController::layoutInvalidated, this, &GcodeController::onLayoutInvalidated);
    }
    setGeneratedGcode(QStringLiteral("; Switch to Handwriting and click Generate G-codes\n"));
    setGcodeStale(true);
}

void GcodeController::onLayoutInvalidated() {
    setGcodeStale(true);
}

void GcodeController::regenerate() {
    if (!m_writer) {
        setGeneratedGcode(QStringLiteral("; No document\n"));
        setGcodeStale(true);
        setPageLineMap({}, 0);
        return;
    }
    const PathBuildResult path = PathBuilder::buildFromController(m_writer);
    const GcodeGenerateResult gen = GcodeGenerator::generateWithPageLines(path, m_writer->settings());
    setGeneratedGcode(gen.gcode);
    setPageLineMap(gen.pageLineStart, gen.pageCount);
    setGcodeStale(false);
}

QString GcodeController::gcodeForPageRange(int startPage, int endPageExclusive) const {
    if (m_generatedGcode.isEmpty() || m_pageCount <= 0 || m_pageLineStart.isEmpty())
        return m_generatedGcode;

    const QStringList allLines = m_generatedGcode.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    if (allLines.isEmpty()) return m_generatedGcode;

    const int start = qBound(0, startPage, m_pageCount - 1);
    const int endEx = qBound(start + 1, endPageExclusive, m_pageCount);

    int lineFrom = 0;
    if (start > 0 && start < m_pageLineStart.size() && m_pageLineStart[start] >= 0)
        lineFrom = m_pageLineStart[start];
    else if (start == 0)
        lineFrom = 0;

    int lineTo = allLines.size();
    if (endEx < m_pageCount && endEx < m_pageLineStart.size() && m_pageLineStart[endEx] >= 0)
        lineTo = m_pageLineStart[endEx];

    lineFrom = qBound(0, lineFrom, allLines.size());
    lineTo = qBound(lineFrom, lineTo, allLines.size());

    QStringList slice;
    for (int i = lineFrom; i < lineTo; ++i)
        slice.append(allLines[i]);

    return slice.join(QLatin1Char('\n')) + QLatin1Char('\n');
}

void GcodeController::copyToClipboard() {
    if (QClipboard *cb = QGuiApplication::clipboard())
        cb->setText(m_generatedGcode);
}

void GcodeController::setGcodeText(const QString &gcode) {
    setGeneratedGcode(gcode);
    setGcodeStale(false);
    setPageLineMap({}, 0);
}

void GcodeController::openGcodeFile() {
    const QString path = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Open G-code"),
        QString(),
        QStringLiteral("G-code (*.gcode);;All files (*)")
    );
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    setGcodeText(QString::fromUtf8(file.readAll()));
    file.close();
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

void GcodeController::setPageLineMap(const QVector<int> &starts, int pageCount) {
    m_pageLineStart = starts;
    m_pageCount = pageCount;
    emit pageLineMapChanged();
}
