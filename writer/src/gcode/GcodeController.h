#pragma once

#include <QObject>
#include <QString>
#include <QVector>

class WriterController;

class GcodeController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString generatedGcode READ generatedGcode WRITE setGcodeText NOTIFY generatedGcodeChanged)
    Q_PROPERTY(bool gcodeStale READ gcodeStale NOTIFY gcodeStaleChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageLineMapChanged)

public:
    explicit GcodeController(WriterController *writer, QObject *parent = nullptr);

    QString generatedGcode() const { return m_generatedGcode; }
    bool gcodeStale() const { return m_gcodeStale; }
    int pageCount() const { return m_pageCount; }
    void setGcodeText(const QString &gcode);

    Q_INVOKABLE void regenerate();
    Q_INVOKABLE void saveGcodeFile();
    Q_INVOKABLE void openGcodeFile();
    Q_INVOKABLE void copyToClipboard();
    Q_INVOKABLE QString gcodeForPageRange(int startPage, int endPageExclusive) const;
    Q_INVOKABLE bool regeneratePage(int pageIndex);

    const QVector<int> &pageLineStart() const { return m_pageLineStart; }

public slots:
    void onLayoutInvalidated();

signals:
    void generatedGcodeChanged();
    void gcodeStaleChanged();
    void pageLineMapChanged();

private:
    void setGeneratedGcode(const QString &gcode);
    void setGcodeStale(bool stale);
    void setPageLineMap(const QVector<int> &starts, int pageCount);

    WriterController *m_writer = nullptr;
    QString m_generatedGcode;
    bool m_gcodeStale = true;
    QVector<int> m_pageLineStart;
    int m_pageCount = 0;
};
