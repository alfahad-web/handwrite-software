#pragma once

#include <QObject>
#include <QString>

class DocumentModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit DocumentModel(QObject *parent = nullptr);

    QString text() const { return m_text; }
    void setText(const QString &t);

signals:
    void textAboutToChange();
    void textChanged();

private:
    QString m_text;
};
