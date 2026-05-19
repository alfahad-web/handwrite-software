#include "GrblConnection.h"

#ifdef WRITER_HAS_SERIALPORT

#include <QSerialPortInfo>
#include <QSettings>

namespace {
constexpr const char *kG = "WriterQt";
constexpr const char *kPortKey = "lastSerialPort";
}

GrblConnection::GrblConnection(QObject *parent) : QObject(parent) {
    m_wakeTimer.setSingleShot(true);
    connect(&m_wakeTimer, &QTimer::timeout, this, &GrblConnection::onWakeTimer);
    connect(&m_serial, &QSerialPort::readyRead, this, &GrblConnection::onReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred, this, &GrblConnection::onSerialError);
    loadLastPort();
    refreshPorts();
}

GrblConnection::~GrblConnection() {
    disconnectPort();
}

void GrblConnection::loadLastPort() {
    QSettings s;
    s.beginGroup(kG);
    m_portName = s.value(kPortKey).toString();
    s.endGroup();
}

void GrblConnection::saveLastPort() {
    QSettings s;
    s.beginGroup(kG);
    s.setValue(kPortKey, m_portName);
    s.endGroup();
}

void GrblConnection::setPortName(const QString &name) {
    if (m_portName == name) return;
    m_portName = name;
    emit portNameChanged();
    saveLastPort();
}

void GrblConnection::refreshPorts() {
    QStringList ports;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        ports.append(info.portName());
    ports.sort();
    if (m_availablePorts == ports) return;
    m_availablePorts = ports;
    emit availablePortsChanged();
    if (!m_portName.isEmpty() && !ports.contains(m_portName) && !ports.isEmpty())
        setPortName(ports.first());
    else if (m_portName.isEmpty() && !ports.isEmpty())
        setPortName(ports.first());
}

void GrblConnection::appendLog(const QString &line, bool fromMachine) {
    const QString prefix = fromMachine ? QStringLiteral("< ") : QStringLiteral("> ");
    const QString entry = prefix + line;
    if (m_consoleLog.isEmpty())
        m_consoleLog = entry;
    else
        m_consoleLog += QLatin1Char('\n') + entry;
    emit consoleLogChanged();
}

void GrblConnection::clearLog() {
    if (m_consoleLog.isEmpty()) return;
    m_consoleLog.clear();
    emit consoleLogChanged();
}

void GrblConnection::logMessage(const QString &msg) {
    if (msg.trimmed().isEmpty()) return;
    appendLog(msg.trimmed(), true);
}

bool GrblConnection::connectPort() {
    if (m_connected) return true;
    refreshPorts();
    if (m_portName.isEmpty()) {
        appendLog(QStringLiteral("No serial port selected."));
        return false;
    }

    m_serial.setPortName(m_portName);
    m_serial.setBaudRate(QSerialPort::Baud115200);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        appendLog(QStringLiteral("Connect failed: %1").arg(m_serial.errorString()));
        return false;
    }

    m_connected = true;
    emit connectedChanged();
    appendLog(QStringLiteral("Connected to %1").arg(m_portName));

    m_waking = true;
    m_waitingOk = false;
    m_sendQueue.clear();
    m_serial.write("\r\n\r\n");
    m_wakeTimer.start(2000);
    return true;
}

void GrblConnection::disconnectPort() {
    cancelStream();
    if (!m_connected && !m_serial.isOpen()) return;
    if (m_serial.isOpen()) m_serial.close();
    m_connected = false;
    m_waking = false;
    m_waitingOk = false;
    m_sendQueue.clear();
    emit connectedChanged();
    appendLog(QStringLiteral("Disconnected."));
}

void GrblConnection::onWakeTimer() {
    m_waking = false;
    m_serial.readAll();
    appendLog(QStringLiteral("GRBL ready."), true);
    trySendNext();
}

QString GrblConnection::stripComment(const QString &line) const {
    int idx = line.indexOf(QLatin1Char(';'));
    QString s = idx >= 0 ? line.left(idx) : line;
    return s.trimmed();
}

void GrblConnection::enqueueLine(const QString &line) {
    const QString stripped = stripComment(line);
    if (stripped.isEmpty()) return;
    m_sendQueue.append(stripped);
}

void GrblConnection::sendLine(const QString &line) {
    if (!m_connected) {
        appendLog(QStringLiteral("Not connected — cannot send."));
        return;
    }
    if (m_waking) {
        appendLog(QStringLiteral("Still waking GRBL — try again."));
        return;
    }
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) return;

    if (trimmed.size() == 1 && (trimmed == QLatin1String("!") || trimmed == QLatin1String("~")
                                || trimmed == QLatin1String("?"))) {
        sendRealtimeCommand(trimmed);
        return;
    }

    appendLog(trimmed, false);
    enqueueLine(trimmed);
    trySendNext();
}

void GrblConnection::sendRealtimeCommand(const QString &cmd) {
    if (!m_connected || !m_serial.isOpen()) {
        appendLog(QStringLiteral("Not connected — cannot send."));
        return;
    }
    const QString c = cmd.trimmed();
    if (c.isEmpty()) return;
    m_serial.write(c.toLatin1());
    appendLog(QStringLiteral("[realtime %1]").arg(c), false);
}

void GrblConnection::streamProgram(const QString &program) {
    if (!m_connected) {
        appendLog(QStringLiteral("Not connected — cannot stream program."));
        emit streamFinished(false);
        return;
    }
    if (m_waking) {
        appendLog(QStringLiteral("Still waking GRBL — try again."));
        emit streamFinished(false);
        return;
    }
    if (m_streaming) cancelStream();

    m_streamLines.clear();
    const QStringList raw = program.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &ln : raw) {
        const QString s = stripComment(ln);
        if (!s.isEmpty()) m_streamLines.append(s);
    }
    if (m_streamLines.isEmpty()) {
        appendLog(QStringLiteral("Program is empty."));
        emit streamFinished(false);
        return;
    }

    m_streaming = true;
    m_streamIndex = 0;
    m_streamTotal = m_streamLines.size();
    m_streamProgress = 0.0;
    emit streamingChanged();
    emit streamProgressChanged();
    appendLog(QStringLiteral("Streaming %1 lines…").arg(m_streamTotal));
    trySendNext();
}

void GrblConnection::cancelStream() {
    if (!m_streaming && m_sendQueue.isEmpty()) return;
    const bool wasStreaming = m_streaming;
    m_streaming = false;
    m_streamLines.clear();
    m_streamIndex = 0;
    m_streamTotal = 0;
    m_streamProgress = 0.0;
    m_sendQueue.clear();
    m_waitingOk = false;
    emit streamingChanged();
    emit streamProgressChanged();
    if (wasStreaming) {
        appendLog(QStringLiteral("Stream cancelled."));
        emit streamFinished(false);
    }
}

void GrblConnection::trySendNext() {
    if (!m_connected || m_waking || m_waitingOk) return;

    QString line;
    if (m_streaming && m_streamIndex < m_streamLines.size()) {
        line = m_streamLines.at(m_streamIndex);
    } else if (!m_sendQueue.isEmpty()) {
        line = m_sendQueue.takeFirst();
    } else {
        if (m_streaming) finishStream(true);
        return;
    }

    m_waitingOk = true;
    m_serial.write((line + QLatin1Char('\n')).toLatin1());
}

void GrblConnection::finishStream(bool success) {
    m_streaming = false;
    m_streamLines.clear();
    m_streamIndex = 0;
    m_streamTotal = 0;
    m_streamProgress = 1.0;
    emit streamingChanged();
    emit streamProgressChanged();
    appendLog(success ? QStringLiteral("Stream finished.") : QStringLiteral("Stream failed."),
              true);
    emit streamFinished(success);
}

void GrblConnection::onReadyRead() {
    while (m_serial.canReadLine()) {
        const QByteArray raw = m_serial.readLine();
        QString line = QString::fromUtf8(raw).trimmed();
        if (line.isEmpty()) continue;
        appendLog(line, true);

        const bool isOk = line == QLatin1String("ok");
        const bool isError = line.startsWith(QLatin1String("error"), Qt::CaseInsensitive)
                             || line.startsWith(QLatin1String("ALARM"), Qt::CaseInsensitive);

        if (m_waitingOk && (isOk || isError)) {
            m_waitingOk = false;
            if (isError && m_streaming) {
                finishStream(false);
                return;
            }
            if (m_streaming && isOk) {
                ++m_streamIndex;
                m_streamProgress = m_streamTotal > 0 ? double(m_streamIndex) / m_streamTotal : 1.0;
                emit streamProgressChanged();
            }
            trySendNext();
        }
    }
}

void GrblConnection::onSerialError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError) return;
    if (error == QSerialPort::ResourceError) {
        appendLog(QStringLiteral("Serial error: %1").arg(m_serial.errorString()));
        disconnectPort();
    }
}

#else // !WRITER_HAS_SERIALPORT

GrblConnection::GrblConnection(QObject *parent) : QObject(parent) {}

GrblConnection::~GrblConnection() = default;

void GrblConnection::setPortName(const QString &name) {
    if (m_portName == name) return;
    m_portName = name;
    emit portNameChanged();
}

void GrblConnection::refreshPorts() {
    if (!m_availablePorts.isEmpty()) return;
    m_availablePorts = QStringList();
    emit availablePortsChanged();
}

void GrblConnection::logMessage(const QString &msg) {
    if (msg.trimmed().isEmpty()) return;
    m_consoleLog = m_consoleLog.isEmpty() ? msg.trimmed() : m_consoleLog + QLatin1Char('\n') + msg.trimmed();
    emit consoleLogChanged();
}

bool GrblConnection::connectPort() {
    logMessage(QStringLiteral("Serial support not built. Install qt6-serialport and rebuild WriterQt."));
    return false;
}

void GrblConnection::disconnectPort() {
    if (!m_connected) return;
    m_connected = false;
    emit connectedChanged();
}

void GrblConnection::sendLine(const QString &) {
    logMessage(QStringLiteral("Serial support not built."));
}

void GrblConnection::streamProgram(const QString &) {
    logMessage(QStringLiteral("Serial support not built."));
    emit streamFinished(false);
}

void GrblConnection::cancelStream() {}

void GrblConnection::clearLog() {
    m_consoleLog.clear();
    emit consoleLogChanged();
}

void GrblConnection::sendRealtimeCommand(const QString &) {
    logMessage(QStringLiteral("Serial support not built."));
}

#endif // WRITER_HAS_SERIALPORT
