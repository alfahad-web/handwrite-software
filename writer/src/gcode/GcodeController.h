#pragma once

#include <QObject>
#include <QString>

class WriterController;

class GcodeController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString generatedGcode READ generatedGcode WRITE setGcodeText NOTIFY generatedGcodeChanged)
    Q_PROPERTY(bool gcodeStale READ gcodeStale NOTIFY gcodeStaleChanged)

public:
    explicit GcodeController(WriterController *writer, QObject *parent = nullptr);

    QString generatedGcode() const { return m_generatedGcode; }
    bool gcodeStale() const { return m_gcodeStale; }
    void setGcodeText(const QString &gcode);

    Q_INVOKABLE void regenerate();
    Q_INVOKABLE void saveGcodeFile();
    Q_INVOKABLE void openGcodeFile();
    Q_INVOKABLE void copyToClipboard();

public slots:
    void onLayoutInvalidated();

signals:
    void generatedGcodeChanged();
    void gcodeStaleChanged();

private:
    void setGeneratedGcode(const QString &gcode);
    void setGcodeStale(bool stale);

    WriterController *m_writer = nullptr;
    QString m_generatedGcode;
    bool m_gcodeStale = true;
};
