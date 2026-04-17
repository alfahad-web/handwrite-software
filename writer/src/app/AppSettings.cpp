#include "AppSettings.h"

#include <cmath>

#include <QSettings>

namespace {
constexpr const char *kG = "WriterQt";

void clampPositive(double &v) {
    if (!std::isfinite(v) || v < 1e-6) v = 1e-6;
}
}

AppSettings::AppSettings(QObject *parent) : QObject(parent) {}

void AppSettings::setFeedRateCmPerS(double v) {
    if (!std::isfinite(v) || v <= 0) v = 0.1;
    if (qFuzzyCompare(m_feedRateCmPerS, v)) return;
    m_feedRateCmPerS = v;
    emit feedRateCmPerSChanged();
    emit anyChanged();
}

void AppSettings::setPageWidthCm(double v) {
    clampPositive(v);
    if (qFuzzyCompare(m_pageWidthCm, v)) return;
    m_pageWidthCm = v;
    emit pageWidthCmChanged();
    emit anyChanged();
}

void AppSettings::setPageHeightCm(double v) {
    clampPositive(v);
    if (qFuzzyCompare(m_pageHeightCm, v)) return;
    m_pageHeightCm = v;
    emit pageHeightCmChanged();
    emit anyChanged();
}

void AppSettings::setLeftMarginCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_leftMarginCm, v)) return;
    m_leftMarginCm = v;
    emit leftMarginCmChanged();
    emit anyChanged();
}

void AppSettings::setRightMarginCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_rightMarginCm, v)) return;
    m_rightMarginCm = v;
    emit rightMarginCmChanged();
    emit anyChanged();
}

void AppSettings::setVerticalGapCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_verticalGapCm, v)) return;
    m_verticalGapCm = v;
    emit verticalGapCmChanged();
    emit anyChanged();
}

void AppSettings::setHxCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_hxCm, v)) return;
    m_hxCm = v;
    emit hxCmChanged();
    emit anyChanged();
}

void AppSettings::setHyCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_hyCm, v)) return;
    m_hyCm = v;
    emit hyCmChanged();
    emit anyChanged();
}

void AppSettings::setLineHeightCm(double v) {
    clampPositive(v);
    if (qFuzzyCompare(m_lineHeightCm, v)) return;
    m_lineHeightCm = v;
    emit lineHeightCmChanged();
    emit anyChanged();
}

void AppSettings::setFontUnitToCm(double v) {
    if (!std::isfinite(v) || v <= 0) v = 1e-6;
    if (qFuzzyCompare(m_fontUnitToCm, v)) return;
    m_fontUnitToCm = v;
    emit fontUnitToCmChanged();
    emit anyChanged();
}

void AppSettings::load() {
    QSettings s;
    s.beginGroup(kG);
    setFeedRateCmPerS(s.value("feedRateCmPerS", m_feedRateCmPerS).toDouble());
    setPageWidthCm(s.value("pageWidthCm", m_pageWidthCm).toDouble());
    setPageHeightCm(s.value("pageHeightCm", m_pageHeightCm).toDouble());
    setLeftMarginCm(s.value("leftMarginCm", m_leftMarginCm).toDouble());
    setRightMarginCm(s.value("rightMarginCm", m_rightMarginCm).toDouble());
    setVerticalGapCm(s.value("verticalGapCm", m_verticalGapCm).toDouble());
    setHxCm(s.value("hxCm", m_hxCm).toDouble());
    setHyCm(s.value("hyCm", m_hyCm).toDouble());
    setLineHeightCm(s.value("lineHeightCm", m_lineHeightCm).toDouble());
    setFontUnitToCm(s.value("fontUnitToCm", m_fontUnitToCm).toDouble());
    s.endGroup();
}

void AppSettings::save() {
    QSettings s;
    s.beginGroup(kG);
    s.setValue("feedRateCmPerS", m_feedRateCmPerS);
    s.setValue("pageWidthCm", m_pageWidthCm);
    s.setValue("pageHeightCm", m_pageHeightCm);
    s.setValue("leftMarginCm", m_leftMarginCm);
    s.setValue("rightMarginCm", m_rightMarginCm);
    s.setValue("verticalGapCm", m_verticalGapCm);
    s.setValue("hxCm", m_hxCm);
    s.setValue("hyCm", m_hyCm);
    s.setValue("lineHeightCm", m_lineHeightCm);
    s.setValue("fontUnitToCm", m_fontUnitToCm);
    s.endGroup();
}
