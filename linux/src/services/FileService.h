#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class FileService : public QObject {
    Q_OBJECT
public:
    explicit FileService(QObject *parent = nullptr);

    Q_INVOKABLE QString selectOrCreateTxtFile();
    Q_INVOKABLE bool appendTxtLines(const QString &filePath, const QStringList &lines, QString *errorMessage = nullptr);
};
