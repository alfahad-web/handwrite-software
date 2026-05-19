#include "SerialPortScan.h"

#include <algorithm>

#include <QDir>
#include <QFileInfo>
#include <QSet>

#ifdef WRITER_HAS_SERIALPORT
#include <QSerialPortInfo>
#endif

QList<SerialPortEntry> SerialPortScan::listPorts() {
#ifdef WRITER_HAS_SERIALPORT
    QList<SerialPortEntry> out;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        SerialPortEntry e;
        e.deviceName = info.portName();
        QString detail = info.description();
        if (detail.isEmpty()) detail = info.manufacturer();
        if (detail.isEmpty() && info.vendorIdentifier() != 0)
            detail = QStringLiteral("USB %1:%2")
                           .arg(info.vendorIdentifier(), 4, 16, QChar('0'))
                           .arg(info.productIdentifier(), 4, 16, QChar('0'));
        if (detail.isEmpty()) detail = QStringLiteral("USB serial");
        e.label = QStringLiteral("%1 — %2").arg(e.deviceName, detail);
        out.append(e);
    }
    std::sort(out.begin(), out.end(), [](const SerialPortEntry &a, const SerialPortEntry &b) {
        return a.deviceName < b.deviceName;
    });
    return out;
#else
    QList<SerialPortEntry> out;
    QSet<QString> seen;

    const auto addDevice = [&](const QString &deviceName, const QString &detail) {
        if (deviceName.isEmpty() || seen.contains(deviceName)) return;
        seen.insert(deviceName);
        SerialPortEntry e;
        e.deviceName = deviceName;
        e.label = detail.isEmpty()
                      ? deviceName
                      : QStringLiteral("%1 — %2").arg(deviceName, detail);
        out.append(e);
    };

    QDir byId(QStringLiteral("/dev/serial/by-id"));
    if (byId.exists()) {
        const QStringList ids = byId.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString &id : ids) {
            const QFileInfo fi(byId.absoluteFilePath(id));
            const QString canonical = fi.canonicalFilePath();
            if (canonical.isEmpty()) continue;
            addDevice(QFileInfo(canonical).fileName(), id);
        }
    }

    QDir dev(QStringLiteral("/dev"));
    const QStringList patterns = {QStringLiteral("ttyACM*"), QStringLiteral("ttyUSB*"),
                                  QStringLiteral("ttyS*")};
    for (const QString &pat : patterns) {
        const QStringList names = dev.entryList({pat}, QDir::System | QDir::Files);
        for (const QString &name : names)
            addDevice(name, QStringLiteral("serial device"));
    }

    std::sort(out.begin(), out.end(), [](const SerialPortEntry &a, const SerialPortEntry &b) {
        return a.deviceName < b.deviceName;
    });
    return out;
#endif
}
