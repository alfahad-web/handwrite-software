#include "DocumentModel.h"

DocumentModel::DocumentModel(QObject *parent) : QObject(parent) {}

void DocumentModel::setText(const QString &t) {
    if (m_text == t) return;
    m_text = t;
    emit textChanged();
}
