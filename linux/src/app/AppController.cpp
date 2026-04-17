#include "AppController.h"

#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QPixmap>

AppController::AppController(EditorStore *store, QObject *parent)
    : QObject(parent), m_store(store) {}

QString AppController::statusMessage() const { return m_statusMessage; }

void AppController::openOrCreateFile() {
    const QString selected = m_fileService.selectOrCreateTxtFile();
    if (selected.isEmpty()) return;
    m_store->setOpenFile(selected);
    m_statusMessage = QStringLiteral("Target file ready.");
    emit statusMessageChanged();
}

bool AppController::canWriteSelection() const {
    return m_store && !m_store->openFilePath().isEmpty() && m_store->selectionRect() != nullptr &&
           !m_store->strokes().isEmpty();
}

bool AppController::writeSelectionInternal() {
    if (!canWriteSelection()) return false;
    const double dpi = ExportService::resolveScreenDpi();
    const auto sampled = ExportService::buildExportStrokes(
        m_store->strokes(),
        m_store->selectionRect(),
        m_store->captureGapUm(),
        dpi
    );
    const QStringList lines = ExportService::serializeExportLines(sampled);
    QString error;
    if (!m_fileService.appendTxtLines(m_store->openFilePath(), lines, &error)) {
        m_statusMessage = QStringLiteral("Write failed: %1").arg(error);
        emit statusMessageChanged();
        return false;
    }
    m_store->markSaved();
    m_statusMessage = QStringLiteral("Selection appended.");
    emit statusMessageChanged();
    return true;
}

void AppController::appendSelection() {
    (void)writeSelectionInternal();
}

void AppController::appendSelectionAndClose() {
    if (m_store->openFilePath().isEmpty()) return;
    (void)writeSelectionInternal();
    m_store->closeFile();
    m_statusMessage = QStringLiteral("File session closed.");
    emit statusMessageChanged();
}

void AppController::setBoardCursorActive(bool active) {
    qInfo() << "[cursor] request active=" << active
            << "prev=" << m_boardCursorActive
            << "toolMode=" << (m_store ? m_store->toolMode() : QStringLiteral("<null-store>"));
    if (active == m_boardCursorActive) return;
    m_boardCursorActive = active;

    if (active) {
        QPixmap pixmap(21, 21);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, false);
        QPen pen(Qt::black, 2);
        painter.setPen(pen);
        painter.drawLine(10, 3, 10, 17);
        painter.drawLine(3, 10, 17, 10);
        painter.end();

        const QCursor plusCursor(pixmap, 10, 10);
        if (QApplication::overrideCursor()) {
            qInfo() << "[cursor] changeOverrideCursor(+)";
            QApplication::changeOverrideCursor(plusCursor);
        } else {
            qInfo() << "[cursor] setOverrideCursor(+)";
            QApplication::setOverrideCursor(plusCursor);
        }
    } else if (QApplication::overrideCursor()) {
        qInfo() << "[cursor] restoreOverrideCursor()";
        QApplication::restoreOverrideCursor();
    } else {
        qInfo() << "[cursor] no override cursor to restore";
    }
}
