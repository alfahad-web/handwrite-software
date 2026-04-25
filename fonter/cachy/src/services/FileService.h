#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class FileService : public QObject {
    Q_OBJECT
public:
    explicit FileService(QObject *parent = nullptr);

    Q_INVOKABLE QString createNewHwFile();
    Q_INVOKABLE QString openHwFile();
    Q_INVOKABLE bool writeTextFileLines(const QString &filePath, const QStringList &lines, QString *errorMessage = nullptr);
    Q_INVOKABLE QString ensureNextFontOutputDir(const QString &projectDir, QString *errorMessage = nullptr);
};
