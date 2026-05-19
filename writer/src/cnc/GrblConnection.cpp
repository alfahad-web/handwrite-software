#include "GrblConnection.h"

#include "SerialPortScan.h"

#include <QSettings>

#if defined(WRITER_POSIX_SERIAL)
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <grp.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace {
constexpr const char *kG = "WriterQt";
constexpr const char *kPortKey = "lastSerialPort";
}

bool GrblConnection::serialAvailable() const {
#ifdef WRITER_HAS_SERIAL
    return true;
#else
    return false;
#endif
}

#ifdef WRITER_HAS_SERIAL

GrblConnection::GrblConnection(QObject *parent) : QObject(parent) {
    m_wakeTimer.setSingleShot(true);
    connect(&m_wakeTimer, &QTimer::timeout, this, &GrblConnection::onWakeTimer);
#ifdef WRITER_HAS_SERIALPORT
    connect(&m_serial, &QSerialPort::readyRead, this, &GrblConnection::onReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred, this, &GrblConnection::onSerialError);
#elif defined(WRITER_POSIX_SERIAL)
    m_readNotifier = new QSocketNotifier(QSocketNotifier::Read, this);
    m_readNotifier->setEnabled(false);
    connect(m_readNotifier, &QSocketNotifier::activated, this, &GrblConnection::onReadyRead);
#endif
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
    QStringList labels;
    for (const SerialPortEntry &e : SerialPortScan::listPorts()) {
        ports.append(e.deviceName);
        labels.append(e.label);
    }
    if (m_availablePorts == ports && m_portLabels == labels) return;
    m_availablePorts = ports;
    m_portLabels = labels;
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

bool GrblConnection::writeRaw(const QByteArray &data) {
#ifdef WRITER_HAS_SERIALPORT
    return m_serial.isOpen() && m_serial.write(data) == data.size();
#elif defined(WRITER_POSIX_SERIAL)
    if (m_fd < 0) return false;
    const ssize_t n = ::write(m_fd, data.constData(), data.size());
    return n == data.size();
#else
    return false;
#endif
}

#if defined(WRITER_POSIX_SERIAL)
namespace {
bool configureTermios(int fd) {
    struct termios tio {};
    if (tcgetattr(fd, &tio) != 0) return false;
    cfmakeraw(&tio);
    cfsetispeed(&tio, B115200);
    cfsetospeed(&tio, B115200);
    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 1;
    return tcsetattr(fd, TCSANOW, &tio) == 0;
}

QString devicePath(const QString &portName) {
    if (portName.startsWith(QLatin1String("/dev/"))) return portName;
    return QStringLiteral("/dev/") + portName;
}

void logOpenFailure(GrblConnection *self, const QString &path) {
    const int err = errno;
    self->logMessage(QStringLiteral("Connect failed: could not open %1 (%2)")
                         .arg(path, QString::fromLocal8Bit(std::strerror(err))));
    if (err == EACCES || err == EPERM) {
        struct stat st {};
        QString groupHint = QStringLiteral("dialout or uucp");
        if (stat(path.toLocal8Bit().constData(), &st) == 0) {
            struct group *gr = getgrgid(st.st_gid);
            if (gr && gr->gr_name) groupHint = QString::fromLocal8Bit(gr->gr_name);
        }
        self->logMessage(
            QStringLiteral("Permission denied. Add your user to group \"%1\", then log out and back in:\n"
                             "  sudo usermod -aG %1 $USER")
                .arg(groupHint));
    } else if (err == EBUSY) {
        self->logMessage(QStringLiteral("Port is in use. Close Universal G-code Sender or any other app using this port."));
    } else if (err == ENOENT) {
        self->logMessage(QStringLiteral("Device not found. Click Refresh and pick the port that appears when the board is plugged in."));
    }
}
}
#endif

bool GrblConnection::connectPort() {
    if (m_connected) return true;
    refreshPorts();
    if (m_portName.isEmpty()) {
        appendLog(QStringLiteral("No serial port selected."));
        return false;
    }

#ifdef WRITER_HAS_SERIALPORT
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
#elif defined(WRITER_POSIX_SERIAL)
    const QString path = devicePath(m_portName);
    if (::access(path.toLocal8Bit().constData(), F_OK) != 0) {
        logOpenFailure(this, path);
        return false;
    }
    m_fd = ::open(path.toLocal8Bit().constData(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd < 0) {
        logOpenFailure(this, path);
        return false;
    }
    const int flags = fcntl(m_fd, F_GETFL, 0);
    if (flags >= 0) fcntl(m_fd, F_SETFL, flags & ~O_NONBLOCK);
    if (!configureTermios(m_fd)) {
        ::close(m_fd);
        m_fd = -1;
        appendLog(QStringLiteral("Connect failed: could not configure %1").arg(path));
        return false;
    }
    m_readNotifier->setSocket(m_fd);
    m_readNotifier->setEnabled(true);
#endif

    m_connected = true;
    emit connectedChanged();
    appendLog(QStringLiteral("Connected to %1").arg(m_portName));

    m_waking = true;
    m_waitingOk = false;
    m_sendQueue.clear();
    m_readBuffer.clear();
    writeRaw("\r\n\r\n");
    m_wakeTimer.start(2000);
    return true;
}

void GrblConnection::disconnectPort() {
    cancelStream();
    if (!m_connected) return;

#ifdef WRITER_HAS_SERIALPORT
    if (m_serial.isOpen()) m_serial.close();
#elif defined(WRITER_POSIX_SERIAL)
    if (m_readNotifier) m_readNotifier->setEnabled(false);
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
#endif

    m_connected = false;
    m_waking = false;
    m_waitingOk = false;
    m_sendQueue.clear();
    m_readBuffer.clear();
    emit connectedChanged();
    appendLog(QStringLiteral("Disconnected."));
}

void GrblConnection::onWakeTimer() {
    m_waking = false;
#ifdef WRITER_HAS_SERIALPORT
    m_serial.readAll();
#else
    m_readBuffer.clear();
    char buf[512];
    while (m_fd >= 0) {
        const ssize_t n = ::read(m_fd, buf, sizeof(buf));
        if (n <= 0) break;
    }
#endif
    appendLog(QStringLiteral("GRBL ready."), true);
    trySendNext();
}

QString GrblConnection::stripComment(const QString &line) const {
    const int idx = line.indexOf(QLatin1Char(';'));
    const QString s = idx >= 0 ? line.left(idx) : line;
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

    if (trimmed.size() == 1
        && (trimmed == QLatin1String("!") || trimmed == QLatin1String("~")
            || trimmed == QLatin1String("?"))) {
        sendRealtimeCommand(trimmed);
        return;
    }

    appendLog(trimmed, false);
    enqueueLine(trimmed);
    trySendNext();
}

void GrblConnection::sendRealtimeCommand(const QString &cmd) {
    if (!m_connected) {
        appendLog(QStringLiteral("Not connected — cannot send."));
        return;
    }
    const QString c = cmd.trimmed();
    if (c.isEmpty()) return;
    writeRaw(c.toLatin1());
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
    writeRaw((line + QLatin1Char('\n')).toUtf8());
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

void GrblConnection::processIncomingData() {
    while (true) {
        const int nl = m_readBuffer.indexOf('\n');
        if (nl < 0) break;

        QByteArray raw = m_readBuffer.left(nl);
        m_readBuffer.remove(0, nl + 1);
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

void GrblConnection::onReadyRead() {
#ifdef WRITER_HAS_SERIALPORT
    m_readBuffer.append(m_serial.readAll());
#elif defined(WRITER_POSIX_SERIAL)
    if (m_fd < 0) return;
    char buf[512];
    const ssize_t n = ::read(m_fd, buf, sizeof(buf));
    if (n > 0) m_readBuffer.append(buf, int(n));
#endif
    processIncomingData();
}

#ifdef WRITER_HAS_SERIALPORT
void GrblConnection::onSerialError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError) return;
    if (error == QSerialPort::ResourceError) {
        appendLog(QStringLiteral("Serial error: %1").arg(m_serial.errorString()));
        disconnectPort();
    }
}
#endif

#else // stub — no serial backend

GrblConnection::GrblConnection(QObject *parent) : QObject(parent) {}

GrblConnection::~GrblConnection() = default;

void GrblConnection::setPortName(const QString &name) {
    if (m_portName == name) return;
    m_portName = name;
    emit portNameChanged();
}

void GrblConnection::refreshPorts() {
    if (m_availablePorts.isEmpty() && m_portLabels.isEmpty()) return;
    m_availablePorts.clear();
    m_portLabels.clear();
    emit availablePortsChanged();
}

void GrblConnection::logMessage(const QString &msg) {
    if (msg.trimmed().isEmpty()) return;
    m_consoleLog = m_consoleLog.isEmpty() ? msg.trimmed() : m_consoleLog + QLatin1Char('\n') + msg.trimmed();
    emit consoleLogChanged();
}

bool GrblConnection::connectPort() {
    logMessage(QStringLiteral("Serial not available. Install qt6-serialport and upgrade qt6-base to match, then rebuild."));
    return false;
}

void GrblConnection::disconnectPort() {
    if (!m_connected) return;
    m_connected = false;
    emit connectedChanged();
}

void GrblConnection::sendLine(const QString &) {
    logMessage(QStringLiteral("Serial not available."));
}

void GrblConnection::streamProgram(const QString &) {
    logMessage(QStringLiteral("Serial not available."));
    emit streamFinished(false);
}

void GrblConnection::cancelStream() {}

void GrblConnection::clearLog() {
    m_consoleLog.clear();
    emit consoleLogChanged();
}

void GrblConnection::sendRealtimeCommand(const QString &) {
    logMessage(QStringLiteral("Serial not available."));
}

#endif // WRITER_HAS_SERIAL
