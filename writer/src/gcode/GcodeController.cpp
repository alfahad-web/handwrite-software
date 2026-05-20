#include "GcodeController.h"

#include "GcodeGenerator.h"
#include "PathBuilder.h"
#include "app/WriterController.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QClipboard>

namespace {
int countProgramLines(const QString &gcode) {
    int count = 0;
    const QStringList raw = gcode.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : raw) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char(';')))
            continue;
        ++count;
    }
    return count;
}
}

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
        setPageLineMap({}, {}, 0);
        return;
    }
    const PathBuildResult path = PathBuilder::buildFromController(m_writer);
    const GcodeGenerateResult gen = GcodeGenerator::generateWithPageLines(path, m_writer->settings());
    setGeneratedGcode(gen.gcode);
    setPageLineMap(gen.pageLineStart, gen.pageLineCount, gen.pageCount);
    setGcodeStale(false);
}

QString GcodeController::gcodeForPageRange(int startPage, int endPageExclusive) const {
    if (!m_writer || !m_writer->settings()) return m_generatedGcode;

    const int start = qMax(0, startPage);
    const int endEx = qMax(start + 1, endPageExclusive);
    const PathBuildResult path = PathBuilder::buildFromController(m_writer);
    const AppSettings *st = m_writer->settings();

    QStringList parts;
    for (int p = start; p < endEx; ++p)
        parts.append(GcodeGenerator::generateSinglePage(path, p, st));

    if (parts.isEmpty()) return QStringLiteral("; No page G-code\n");
    return parts.join(QLatin1Char('\n'));
}

bool GcodeController::regeneratePage(int pageIndex) {
    if (!m_writer || !m_writer->settings()) {
        setGeneratedGcode(QStringLiteral("; No document\n"));
        setGcodeStale(true);
        return false;
    }

    const PathBuildResult path = PathBuilder::buildFromController(m_writer);
    const int clampedPage = qMax(0, pageIndex);
    const QString pageGcode =
        GcodeGenerator::generateSinglePage(path, clampedPage, m_writer->settings());
    setGeneratedGcode(pageGcode);
    setPageLineMap({7}, {countProgramLines(pageGcode)}, 1);
    setGcodeStale(false);
    return !pageGcode.startsWith(QLatin1String("; No strokes on page"));
}

void GcodeController::copyToClipboard() {
    if (QClipboard *cb = QGuiApplication::clipboard())
        cb->setText(m_generatedGcode);
}

void GcodeController::setGcodeText(const QString &gcode) {
    setGeneratedGcode(gcode);
    setGcodeStale(false);
    setPageLineMap({}, {}, 0);
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

int GcodeController::pageProgramLineCount(int pageIndex) const {
    if (pageIndex < 0 || pageIndex >= m_pageLineCount.size()) return 0;
    return m_pageLineCount.at(pageIndex);
}

void GcodeController::setPageLineMap(const QVector<int> &starts, const QVector<int> &counts, int pageCount) {
    m_pageLineStart = starts;
    m_pageLineCount = counts;
    m_pageCount = pageCount;
    emit pageLineMapChanged();
}
