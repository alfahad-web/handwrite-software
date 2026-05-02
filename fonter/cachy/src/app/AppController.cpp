#include "AppController.h"

#include "../core/EditorTypes.h"

#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <algorithm>

AppController::AppController(EditorStore *store, QObject *parent)
    : QObject(parent), m_store(store) {}

QString AppController::statusMessage() const { return m_statusMessage; }

void AppController::newProject() {
    const QString selected = m_fileService.createNewHwFile();
    if (selected.isEmpty()) return;
    m_store->clearAll();
    m_store->setProjectFilePath(selected);
    m_statusMessage = QStringLiteral("New project created.");
    emit statusMessageChanged();
}

void AppController::openProject() {
    const QString selected = m_fileService.openHwFile();
    if (selected.isEmpty()) return;
    QString error;
    if (!m_projectService.loadProject(selected, m_store, &error)) {
        m_statusMessage = QStringLiteral("Open failed: %1").arg(error);
    } else {
        m_statusMessage = QStringLiteral("Project loaded.");
    }
    emit statusMessageChanged();
}

void AppController::saveProject() {
    if (!m_store) return;
    QString path = m_store->projectFilePath();
    if (path.isEmpty()) path = m_fileService.createNewHwFile();
    if (path.isEmpty()) return;
    QString error;
    if (!m_projectService.saveProject(path, *m_store, &error)) {
        m_statusMessage = QStringLiteral("Save failed: %1").arg(error);
    } else {
        m_store->setProjectFilePath(path);
        m_store->markSaved();
        m_statusMessage = QStringLiteral("Project saved.");
    }
    emit statusMessageChanged();
}

void AppController::assignSelectionCharacter(const QString &selectionId, const QString &text, const QString &joinMode) {
    if (!m_store) return;
    if (text.size() != 1) {
        m_statusMessage = QStringLiteral("Assignment must be one ASCII character.");
        emit statusMessageChanged();
        return;
    }
    const QChar ch = text[0];
    const ushort code = ch.unicode();
    if (code > 127) {
        m_statusMessage = QStringLiteral("Only ASCII characters are allowed.");
        emit statusMessageChanged();
        return;
    }
    if (joinMode != QStringLiteral("L") && joinMode != QStringLiteral("R") && joinMode != QStringLiteral("LR")
        && joinMode != QStringLiteral("N")) {
        m_statusMessage = QStringLiteral("Join mode must be L, R, LR, or N.");
        emit statusMessageChanged();
        return;
    }
    SelectionBox *box = m_store->selectionByIdMutable(selectionId);
    if (!box) return;
    box->assigned = true;
    box->assignedAscii = static_cast<int>(code);
    // Persist the user's entered character as-is; export naming is derived
    // from ASCII code at generation time for deterministic filenames.
    box->fileStem = QString(ch);
    box->joinMode = joinModeFromString(joinMode);
    m_store->recomputeSelectionAnchors();
    m_store->markDirty();
    m_statusMessage = QStringLiteral("Selection assigned.");
    emit statusMessageChanged();
}

void AppController::deleteSelectedSelection() {
    if (!m_store) return;
    if (m_store->deleteSelectedSelection()) {
        m_statusMessage = QStringLiteral("Selection deleted.");
    } else {
        m_statusMessage = QStringLiteral("No active selection.");
    }
    emit statusMessageChanged();
}

void AppController::generateFonts() {
    if (!m_store) return;
    if (m_store->projectFilePath().isEmpty()) {
        m_statusMessage = QStringLiteral("Save project first.");
        emit statusMessageChanged();
        return;
    }
    QString error;
    const QString outDir = m_fileService.ensureNextFontOutputDir(QFileInfo(m_store->projectFilePath()).absolutePath(), &error);
    if (outDir.isEmpty()) {
        m_statusMessage = QStringLiteral("Generate failed: %1").arg(error);
        emit statusMessageChanged();
        return;
    }
    m_store->recomputeSelectionAnchors();
    const double dpi = ExportService::resolveScreenDpi();
    auto boxes = m_store->selectionBoxes();
    std::sort(boxes.begin(), boxes.end(), [](const SelectionBox &a, const SelectionBox &b) {
        return a.orderIndex < b.orderIndex;
    });
    QSet<QString> invalidSelectionIds;
    QString firstInvalidSelectionId;
    for (const SelectionBox &box : boxes) {
        const QString join = joinModeToString(box.joinMode);
        const bool joinValid = join == QStringLiteral("L")
            || join == QStringLiteral("R")
            || join == QStringLiteral("LR")
            || join == QStringLiteral("N");
        const bool valid =
            box.assigned
            && box.assignedAscii >= 0
            && box.assignedAscii <= 127
            && joinValid;
        if (!valid) {
            invalidSelectionIds.insert(box.id);
            if (firstInvalidSelectionId.isEmpty()) {
                firstInvalidSelectionId = box.id;
            }
        }
    }
    if (!invalidSelectionIds.isEmpty()) {
        m_store->setHighlightedSelectionIds(invalidSelectionIds);
        if (!firstInvalidSelectionId.isEmpty()) {
            m_store->setSelectedSelectionId(firstInvalidSelectionId);
        }
        const int count = invalidSelectionIds.size();
        m_statusMessage = QStringLiteral("%1 selection boxes have incompatible details.")
                              .arg(count);
        emit statusMessageChanged();
        QMessageBox::warning(
            nullptr,
            QStringLiteral("Invalid Selection Details"),
            m_statusMessage
        );
        return;
    }
    m_store->clearHighlightedSelectionIds();
    QHash<QString, int> stemCounter;
    int generated = 0;
    int skipped = 0;
    for (const SelectionBox &box : boxes) {
        if (!box.assigned || box.assignedAscii < 0 || box.assignedAscii > 127) {
            ++skipped;
            continue;
        }
        const QString deterministicStem = m_store->fileStemForAscii(box.assignedAscii);
        const QString stemJoin = QStringLiteral("%1.%2").arg(deterministicStem).arg(joinModeToString(box.joinMode));
        const int seq = stemCounter.value(stemJoin, 0) + 1;
        stemCounter.insert(stemJoin, seq);
        SelectionBox exportBox = box;
        exportBox.fileStem = QStringLiteral("%1.%2.%3")
                                 .arg(deterministicStem)
                                 .arg(joinModeToString(box.joinMode))
                                 .arg(seq);
        QVector<Stroke> maskedStrokes = m_store->strokes();
        for (Stroke &stroke : maskedStrokes) {
            for (int pointIndex = 0; pointIndex < stroke.points.size(); ++pointIndex) {
                if (m_store->isPointErasedInSelection(box.id, stroke.id, pointIndex)) {
                    stroke.points[pointIndex].erased = true;
                }
            }
        }
        const auto files = ExportService::buildSelectionExports(
            maskedStrokes,
            QVector<SelectionBox>{exportBox},
            m_store->captureGapUm(),
            dpi
        );
        for (const SelectionExportFile &f : files) {
            if (f.lines.isEmpty()) continue;
            if (m_fileService.writeTextFileLines(QDir(outDir).absoluteFilePath(f.fileName), f.lines, &error)) {
                ++generated;
            }
        }
    }
    m_statusMessage = QStringLiteral("Generated %1 files (%2 skipped) in %3").arg(generated).arg(skipped).arg(outDir);
    emit statusMessageChanged();
}

void AppController::setBoardCursorActive(bool active) {
    qInfo() << "[cursor] request active=" << active
            << "prev=" << m_boardCursorActive
            << "toolMode=" << (m_store ? m_store->toolMode() : QStringLiteral("<null-store>"));
    if (active == m_boardCursorActive) return;
    m_boardCursorActive = active;

    if (active) {
        if (m_eraseCursorActive) {
            m_eraseCursorActive = false;
        }
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

void AppController::setEraseCursorActive(bool active, int radiusPx, int zoomPercent) {
    if (active == m_eraseCursorActive && active) {
        // Keep cursor size synced with runtime radius/zoom changes.
    } else if (active == m_eraseCursorActive) {
        return;
    }
    m_eraseCursorActive = active;
    if (active) {
        if (m_boardCursorActive) m_boardCursorActive = false;
        const qreal scale = qMax(0.1, zoomPercent / 100.0);
        const int radius = qMax(2, static_cast<int>(qRound(radiusPx * scale)));
        const int size = radius * 2 + 8;
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(15, 23, 42, 190), 1.5));
        painter.setBrush(QColor(59, 130, 246, 70));
        painter.drawEllipse(QPointF(size / 2.0, size / 2.0), radius, radius);
        painter.end();
        const QCursor eraseCursor(pixmap, size / 2, size / 2);
        if (QApplication::overrideCursor()) {
            QApplication::changeOverrideCursor(eraseCursor);
        } else {
            QApplication::setOverrideCursor(eraseCursor);
        }
        return;
    }
    if (QApplication::overrideCursor()) {
        QApplication::restoreOverrideCursor();
    }
}
