#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#ifdef WRITER_HAS_SERIALPORT
#include <QSerialPort>
#include <QTimer>
#endif

class GrblConnection : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString consoleLog READ consoleLog NOTIFY consoleLogChanged)
    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY availablePortsChanged)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(bool streaming READ streaming NOTIFY streamingChanged)
    Q_PROPERTY(double streamProgress READ streamProgress NOTIFY streamProgressChanged)

public:
    explicit GrblConnection(QObject *parent = nullptr);
    ~GrblConnection() override;

    bool connected() const { return m_connected; }
    QString consoleLog() const { return m_consoleLog; }
    QStringList availablePorts() const { return m_availablePorts; }
    QString portName() const { return m_portName; }
    void setPortName(const QString &name);
    bool streaming() const { return m_streaming; }
    double streamProgress() const { return m_streamProgress; }

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
    QString m_consoleLog;
    QStringList m_availablePorts;
    QString m_portName;
    bool m_connected = false;
    bool m_streaming = false;
    double m_streamProgress = 0.0;

#ifdef WRITER_HAS_SERIALPORT
    void appendLog(const QString &line, bool fromMachine = false);
    void loadLastPort();
    void saveLastPort();
    QString stripComment(const QString &line) const;
    void enqueueLine(const QString &line);
    void finishStream(bool success);
    void trySendNext();

    QSerialPort m_serial;
    QTimer m_wakeTimer;
    bool m_waitingOk = false;
    bool m_waking = false;
    QStringList m_sendQueue;
    QStringList m_streamLines;
    int m_streamIndex = 0;
    int m_streamTotal = 0;

private slots:
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);
    void onWakeTimer();
#endif
};
