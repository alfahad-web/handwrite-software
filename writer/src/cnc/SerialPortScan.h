#pragma once

#include <QStringList>

struct SerialPortEntry {
    QString deviceName;
    QString label;
};

class SerialPortScan {
public:
    static QList<SerialPortEntry> listPorts();
};
