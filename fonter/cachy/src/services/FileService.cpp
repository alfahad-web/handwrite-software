#include "FileService.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>

FileService::FileService(QObject *parent) : QObject(parent) {}

QString FileService::createNewHwFile() {
    const QString selected = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Create .hw file"),
        QString(),
        QStringLiteral("Handwrite Project (*.hw)")
    );
    return selected;
}

QString FileService::openHwFile() {
    return QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Open .hw file"),
        QString(),
        QStringLiteral("Handwrite Project (*.hw)")
    );
}

bool FileService::writeTextFileLines(const QString &filePath, const QStringList &lines, QString *errorMessage) {
    if (filePath.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("File path is empty.");
        return false;
    }
    if (lines.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("No lines to append.");
        return false;
    }

    QFile out(filePath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage) *errorMessage = out.errorString();
        return false;
    }

    QTextStream stream(&out);
    for (const QString &line : lines) {
        const QString clean = line.trimmed();
        if (clean.isEmpty()) continue;
        stream << clean << '\n';
    }
    stream.flush();
    return true;
}

QString FileService::ensureNextFontOutputDir(const QString &projectDir, QString *errorMessage) {
    if (projectDir.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("Project directory is empty.");
        return QString();
    }
    QDir dir(projectDir);
    if (!dir.exists()) {
        if (errorMessage) *errorMessage = QStringLiteral("Project directory does not exist.");
        return QString();
    }
    QString base = "font";
    QString next = base;
    int idx = 0;
    while (dir.exists(next)) {
        ++idx;
        next = QString("font%1").arg(idx);
    }
    if (!dir.mkpath(next)) {
        if (errorMessage) *errorMessage = QStringLiteral("Failed to create output directory.");
        return QString();
    }
    return dir.absoluteFilePath(next);
}
