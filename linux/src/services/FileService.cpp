#include "FileService.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>

FileService::FileService(QObject *parent) : QObject(parent) {}

QString FileService::selectOrCreateTxtFile() {
    const QString selected = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Open or Create .txt file"),
        QString(),
        QStringLiteral("Text Files (*.txt)")
    );
    return selected;
}

bool FileService::appendTxtLines(const QString &filePath, const QStringList &lines, QString *errorMessage) {
    if (filePath.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("File path is empty.");
        return false;
    }
    if (lines.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("No lines to append.");
        return false;
    }

    QFile out(filePath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
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
