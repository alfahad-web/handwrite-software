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
    emit aboutToChange();
    m_feedRateCmPerS = v;
    emit feedRateCmPerSChanged();
    emit feedRateMmPerMinChanged();
    emit anyChanged();
}

void AppSettings::setPageWidthCm(double v) {
    clampPositive(v);
    if (qFuzzyCompare(m_pageWidthCm, v)) return;
    emit aboutToChange();
    m_pageWidthCm = v;
    emit pageWidthCmChanged();
    emit anyChanged();
}

void AppSettings::setPageHeightCm(double v) {
    clampPositive(v);
    if (qFuzzyCompare(m_pageHeightCm, v)) return;
    emit aboutToChange();
    m_pageHeightCm = v;
    emit pageHeightCmChanged();
    emit anyChanged();
}

void AppSettings::setLeftMarginCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_leftMarginCm, v)) return;
    emit aboutToChange();
    m_leftMarginCm = v;
    emit leftMarginCmChanged();
    emit anyChanged();
}

void AppSettings::setRightMarginCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_rightMarginCm, v)) return;
    emit aboutToChange();
    m_rightMarginCm = v;
    emit rightMarginCmChanged();
    emit anyChanged();
}

void AppSettings::setVerticalGapCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_verticalGapCm, v)) return;
    emit aboutToChange();
    m_verticalGapCm = v;
    emit verticalGapCmChanged();
    emit anyChanged();
}

void AppSettings::setHxCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_hxCm, v)) return;
    emit aboutToChange();
    m_hxCm = v;
    emit hxCmChanged();
    emit anyChanged();
}

void AppSettings::setHyCm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_hyCm, v)) return;
    emit aboutToChange();
    m_hyCm = v;
    emit hyCmChanged();
    emit anyChanged();
}

void AppSettings::setLineHeightCm(double v) {
    clampPositive(v);
    if (qFuzzyCompare(m_lineHeightCm, v)) return;
    emit aboutToChange();
    m_lineHeightCm = v;
    emit lineHeightCmChanged();
    emit anyChanged();
}

void AppSettings::setFontUnitToCm(double v) {
    if (!std::isfinite(v) || v <= 0) v = 1e-6;
    if (qFuzzyCompare(m_fontUnitToCm, v)) return;
    emit aboutToChange();
    m_fontUnitToCm = v;
    emit fontUnitToCmChanged();
    emit anyChanged();
}

void AppSettings::setJoinDistMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_joinDistMm, v)) return;
    emit aboutToChange();
    m_joinDistMm = v;
    emit joinDistMmChanged();
    emit anyChanged();
}

void AppSettings::setBacklashYStartMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_backlashYStartMm, v)) return;
    emit aboutToChange();
    m_backlashYStartMm = v;
    emit backlashYStartMmChanged();
    emit anyChanged();
}

void AppSettings::setBacklashYEndMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_backlashYEndMm, v)) return;
    emit aboutToChange();
    m_backlashYEndMm = v;
    emit backlashYEndMmChanged();
    emit anyChanged();
}

void AppSettings::setXErrorNearMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_xErrorNearMm, v)) return;
    emit aboutToChange();
    m_xErrorNearMm = v;
    emit xErrorNearMmChanged();
    emit anyChanged();
}

void AppSettings::setXErrorMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_xErrorMm, v)) return;
    emit aboutToChange();
    m_xErrorMm = v;
    emit xErrorMmChanged();
    emit anyChanged();
}

void AppSettings::setYErrorNearMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_yErrorNearMm, v)) return;
    emit aboutToChange();
    m_yErrorNearMm = v;
    emit yErrorNearMmChanged();
    emit anyChanged();
}

void AppSettings::setYErrorMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_yErrorMm, v)) return;
    emit aboutToChange();
    m_yErrorMm = v;
    emit yErrorMmChanged();
    emit anyChanged();
}

void AppSettings::setSimplifyToleranceMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_simplifyToleranceMm, v)) return;
    emit aboutToChange();
    m_simplifyToleranceMm = v;
    emit simplifyToleranceMmChanged();
    emit anyChanged();
}

void AppSettings::setMinSegmentMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_minSegmentMm, v)) return;
    emit aboutToChange();
    m_minSegmentMm = v;
    emit minSegmentMmChanged();
    emit anyChanged();
}

void AppSettings::setCollinearToleranceMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_collinearToleranceMm, v)) return;
    emit aboutToChange();
    m_collinearToleranceMm = v;
    emit collinearToleranceMmChanged();
    emit anyChanged();
}

void AppSettings::setStreamingPreset(const QString &v) {
    QString normalized = v.trimmed().toLower();
    if (normalized != QLatin1String("safe")
        && normalized != QLatin1String("balanced")
        && normalized != QLatin1String("fast")) {
        normalized = QStringLiteral("balanced");
    }
    if (m_streamingPreset == normalized) return;
    emit aboutToChange();
    m_streamingPreset = normalized;
    emit streamingPresetChanged();
    emit anyChanged();
}

void AppSettings::setArcFitEnabled(bool v) {
    if (m_arcFitEnabled == v) return;
    emit aboutToChange();
    m_arcFitEnabled = v;
    emit arcFitEnabledChanged();
    emit anyChanged();
}

void AppSettings::setArcFitToleranceMm(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_arcFitToleranceMm, v)) return;
    emit aboutToChange();
    m_arcFitToleranceMm = v;
    emit arcFitToleranceMmChanged();
    emit anyChanged();
}

void AppSettings::setGrblJunctionDeviation(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_grblJunctionDeviation, v)) return;
    emit aboutToChange();
    m_grblJunctionDeviation = v;
    emit grblJunctionDeviationChanged();
    emit anyChanged();
}

void AppSettings::setGrblAccelX(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_grblAccelX, v)) return;
    emit aboutToChange();
    m_grblAccelX = v;
    emit grblAccelXChanged();
    emit anyChanged();
}

void AppSettings::setGrblAccelY(double v) {
    if (!std::isfinite(v) || v < 0) v = 0;
    if (qFuzzyCompare(m_grblAccelY, v)) return;
    emit aboutToChange();
    m_grblAccelY = v;
    emit grblAccelYChanged();
    emit anyChanged();
}

void AppSettings::setPenUpZ(double v) {
    if (!std::isfinite(v)) v = 30.0;
    if (qFuzzyCompare(m_penUpZ, v)) return;
    emit aboutToChange();
    m_penUpZ = v;
    emit penUpZChanged();
    emit anyChanged();
}

void AppSettings::setPenDownZ(double v) {
    if (!std::isfinite(v)) v = -5.0;
    if (qFuzzyCompare(m_penDownZ, v)) return;
    emit aboutToChange();
    m_penDownZ = v;
    emit penDownZChanged();
    emit anyChanged();
}

void AppSettings::setPreviewDisplayScale(double v) {
    if (!std::isfinite(v) || v < 0.25) v = 0.25;
    if (v > 3.0) v = 3.0;
    if (qFuzzyCompare(m_previewDisplayScale, v)) return;
    emit aboutToChange();
    m_previewDisplayScale = v;
    emit previewDisplayScaleChanged();
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
    setJoinDistMm(s.value("joinDistMm", m_joinDistMm).toDouble());
    setBacklashYStartMm(s.value("backlashYStartMm", m_backlashYStartMm).toDouble());
    setBacklashYEndMm(s.value("backlashYEndMm", m_backlashYEndMm).toDouble());
    setXErrorNearMm(s.value("xErrorNearMm", m_xErrorNearMm).toDouble());
    setXErrorMm(s.value("xErrorMm", m_xErrorMm).toDouble());
    setYErrorNearMm(s.value("yErrorNearMm", m_yErrorNearMm).toDouble());
    setYErrorMm(s.value("yErrorMm", m_yErrorMm).toDouble());
    setSimplifyToleranceMm(s.value("simplifyToleranceMm", m_simplifyToleranceMm).toDouble());
    setMinSegmentMm(s.value("minSegmentMm", m_minSegmentMm).toDouble());
    setCollinearToleranceMm(s.value("collinearToleranceMm", m_collinearToleranceMm).toDouble());
    setStreamingPreset(s.value("streamingPreset", m_streamingPreset).toString());
    setArcFitEnabled(s.value("arcFitEnabled", m_arcFitEnabled).toBool());
    setArcFitToleranceMm(s.value("arcFitToleranceMm", m_arcFitToleranceMm).toDouble());
    setGrblJunctionDeviation(s.value("grblJunctionDeviation", m_grblJunctionDeviation).toDouble());
    setGrblAccelX(s.value("grblAccelX", m_grblAccelX).toDouble());
    setGrblAccelY(s.value("grblAccelY", m_grblAccelY).toDouble());
    setPenUpZ(s.value("penUpZ", m_penUpZ).toDouble());
    setPenDownZ(s.value("penDownZ", m_penDownZ).toDouble());
    setPreviewDisplayScale(s.value("previewDisplayScale", m_previewDisplayScale).toDouble());
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
    s.setValue("joinDistMm", m_joinDistMm);
    s.setValue("backlashYStartMm", m_backlashYStartMm);
    s.setValue("backlashYEndMm", m_backlashYEndMm);
    s.setValue("xErrorNearMm", m_xErrorNearMm);
    s.setValue("xErrorMm", m_xErrorMm);
    s.setValue("yErrorNearMm", m_yErrorNearMm);
    s.setValue("yErrorMm", m_yErrorMm);
    s.setValue("simplifyToleranceMm", m_simplifyToleranceMm);
    s.setValue("minSegmentMm", m_minSegmentMm);
    s.setValue("collinearToleranceMm", m_collinearToleranceMm);
    s.setValue("streamingPreset", m_streamingPreset);
    s.setValue("arcFitEnabled", m_arcFitEnabled);
    s.setValue("arcFitToleranceMm", m_arcFitToleranceMm);
    s.setValue("grblJunctionDeviation", m_grblJunctionDeviation);
    s.setValue("grblAccelX", m_grblAccelX);
    s.setValue("grblAccelY", m_grblAccelY);
    s.setValue("penUpZ", m_penUpZ);
    s.setValue("penDownZ", m_penDownZ);
    s.setValue("previewDisplayScale", m_previewDisplayScale);
    s.endGroup();
}
