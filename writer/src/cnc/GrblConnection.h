#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#if defined(WRITER_HAS_SERIALPORT) || defined(WRITER_POSIX_SERIAL)
#define WRITER_HAS_SERIAL 1
#endif

#ifdef WRITER_HAS_SERIALPORT
#include <QSerialPort>
#endif
#ifdef WRITER_POSIX_SERIAL
#include <QSocketNotifier>
#endif
#include <QTimer>

class GrblConnection : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString consoleLog READ consoleLog NOTIFY consoleLogChanged)
    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY availablePortsChanged)
    Q_PROPERTY(QStringList portLabels READ portLabels NOTIFY availablePortsChanged)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(bool streaming READ streaming NOTIFY streamingChanged)
    Q_PROPERTY(double streamProgress READ streamProgress NOTIFY streamProgressChanged)
    Q_PROPERTY(bool serialAvailable READ serialAvailable CONSTANT)

public:
    explicit GrblConnection(QObject *parent = nullptr);
    ~GrblConnection() override;

    bool connected() const { return m_connected; }
    QString consoleLog() const { return m_consoleLog; }
    QStringList availablePorts() const { return m_availablePorts; }
    QStringList portLabels() const { return m_portLabels; }
    QString portName() const { return m_portName; }
    void setPortName(const QString &name);
    bool streaming() const { return m_streaming; }
    double streamProgress() const { return m_streamProgress; }
    bool serialAvailable() const;

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE bool connectPort();
    Q_INVOKABLE void disconnectPort();
    Q_INVOKABLE void sendLine(const QString &line);
    Q_INVOKABLE void streamProgram(const QString &program);
    Q_INVOKABLE void cancelStream();
    Q_INVOKABLE void clearLog();
    Q_INVOKABLE void logMessage(const QString &msg);
    Q_INVOKABLE void sendRealtimeCommand(const QString &cmd);

signals:
    void connectedChanged();
    void consoleLogChanged();
    void availablePortsChanged();
    void portNameChanged();
    void streamingChanged();
    void streamProgressChanged();
    void streamFinished(bool success);

private:
#ifdef WRITER_HAS_SERIAL
    void appendLog(const QString &line, bool fromMachine = false);
    void loadLastPort();
    void saveLastPort();
    QString stripComment(const QString &line) const;
    void enqueueLine(const QString &line);
    void finishStream(bool success);
    void trySendNext();
    void processIncomingData();
    bool writeRaw(const QByteArray &data);

    QString m_consoleLog;
    QStringList m_availablePorts;
    QStringList m_portLabels;
    QString m_portName;
    bool m_connected = false;
    bool m_streaming = false;
    double m_streamProgress = 0.0;
    bool m_waitingOk = false;
    bool m_waking = false;
    QStringList m_sendQueue;
    QStringList m_streamLines;
    int m_streamIndex = 0;
    int m_streamTotal = 0;
    QTimer m_wakeTimer;
    QByteArray m_readBuffer;

#ifdef WRITER_HAS_SERIALPORT
    QSerialPort m_serial;
private slots:
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);
#elif defined(WRITER_POSIX_SERIAL)
    int m_fd = -1;
    QSocketNotifier *m_readNotifier = nullptr;
private slots:
    void onReadyRead();
#endif
    void onWakeTimer();
#else
    QString m_consoleLog;
    QStringList m_availablePorts;
    QStringList m_portLabels;
    QString m_portName;
    bool m_connected = false;
    bool m_streaming = false;
    double m_streamProgress = 0.0;
#endif
};
