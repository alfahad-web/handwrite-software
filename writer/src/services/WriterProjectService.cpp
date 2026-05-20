#include "WriterProjectService.h"

#include "app/WriterController.h"
#include "app/AppSettings.h"
#include "app/DocumentModel.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr int kFormatVersion = 3;

QJsonObject settingsToJson(const AppSettings *st) {
    QJsonObject o;
    o.insert(QStringLiteral("feedRateCmPerS"), st->feedRateCmPerS());
    o.insert(QStringLiteral("pageWidthCm"), st->pageWidthCm());
    o.insert(QStringLiteral("pageHeightCm"), st->pageHeightCm());
    o.insert(QStringLiteral("leftMarginCm"), st->leftMarginCm());
    o.insert(QStringLiteral("rightMarginCm"), st->rightMarginCm());
    o.insert(QStringLiteral("verticalGapCm"), st->verticalGapCm());
    o.insert(QStringLiteral("hxCm"), st->hxCm());
    o.insert(QStringLiteral("hyCm"), st->hyCm());
    o.insert(QStringLiteral("lineHeightCm"), st->lineHeightCm());
    o.insert(QStringLiteral("fontUnitToCm"), st->fontUnitToCm());
    o.insert(QStringLiteral("joinDistMm"), st->joinDistMm());
    o.insert(QStringLiteral("backlashYStartMm"), st->backlashYStartMm());
    o.insert(QStringLiteral("backlashYEndMm"), st->backlashYEndMm());
    o.insert(QStringLiteral("xErrorNearMm"), st->xErrorNearMm());
    o.insert(QStringLiteral("xErrorMm"), st->xErrorMm());
    o.insert(QStringLiteral("yErrorNearMm"), st->yErrorNearMm());
    o.insert(QStringLiteral("yErrorMm"), st->yErrorMm());
    o.insert(QStringLiteral("simplifyToleranceMm"), st->simplifyToleranceMm());
    o.insert(QStringLiteral("minSegmentMm"), st->minSegmentMm());
    o.insert(QStringLiteral("collinearToleranceMm"), st->collinearToleranceMm());
    o.insert(QStringLiteral("streamingPreset"), st->streamingPreset());
    o.insert(QStringLiteral("arcFitEnabled"), st->arcFitEnabled());
    o.insert(QStringLiteral("arcFitToleranceMm"), st->arcFitToleranceMm());
    o.insert(QStringLiteral("grblJunctionDeviation"), st->grblJunctionDeviation());
    o.insert(QStringLiteral("grblAccelX"), st->grblAccelX());
    o.insert(QStringLiteral("grblAccelY"), st->grblAccelY());
    o.insert(QStringLiteral("penUpZ"), st->penUpZ());
    o.insert(QStringLiteral("penDownZ"), st->penDownZ());
    o.insert(QStringLiteral("previewDisplayScale"), st->previewDisplayScale());
    return o;
}

void applySettingsFromJson(AppSettings *st, const QJsonObject &o) {
    if (o.contains(QStringLiteral("feedRateCmPerS"))) st->setFeedRateCmPerS(o.value(QStringLiteral("feedRateCmPerS")).toDouble());
    if (o.contains(QStringLiteral("pageWidthCm"))) st->setPageWidthCm(o.value(QStringLiteral("pageWidthCm")).toDouble());
    if (o.contains(QStringLiteral("pageHeightCm"))) st->setPageHeightCm(o.value(QStringLiteral("pageHeightCm")).toDouble());
    if (o.contains(QStringLiteral("leftMarginCm"))) st->setLeftMarginCm(o.value(QStringLiteral("leftMarginCm")).toDouble());
    if (o.contains(QStringLiteral("rightMarginCm"))) st->setRightMarginCm(o.value(QStringLiteral("rightMarginCm")).toDouble());
    if (o.contains(QStringLiteral("verticalGapCm"))) st->setVerticalGapCm(o.value(QStringLiteral("verticalGapCm")).toDouble());
    if (o.contains(QStringLiteral("hxCm"))) st->setHxCm(o.value(QStringLiteral("hxCm")).toDouble());
    if (o.contains(QStringLiteral("hyCm"))) st->setHyCm(o.value(QStringLiteral("hyCm")).toDouble());
    if (o.contains(QStringLiteral("lineHeightCm"))) st->setLineHeightCm(o.value(QStringLiteral("lineHeightCm")).toDouble());
    if (o.contains(QStringLiteral("fontUnitToCm"))) st->setFontUnitToCm(o.value(QStringLiteral("fontUnitToCm")).toDouble());
    if (o.contains(QStringLiteral("joinDistMm"))) st->setJoinDistMm(o.value(QStringLiteral("joinDistMm")).toDouble());
    if (o.contains(QStringLiteral("backlashYStartMm"))) st->setBacklashYStartMm(o.value(QStringLiteral("backlashYStartMm")).toDouble());
    if (o.contains(QStringLiteral("backlashYEndMm"))) st->setBacklashYEndMm(o.value(QStringLiteral("backlashYEndMm")).toDouble());
    if (o.contains(QStringLiteral("xErrorNearMm"))) st->setXErrorNearMm(o.value(QStringLiteral("xErrorNearMm")).toDouble());
    if (o.contains(QStringLiteral("xErrorMm"))) st->setXErrorMm(o.value(QStringLiteral("xErrorMm")).toDouble());
    if (o.contains(QStringLiteral("yErrorNearMm"))) st->setYErrorNearMm(o.value(QStringLiteral("yErrorNearMm")).toDouble());
    if (o.contains(QStringLiteral("yErrorMm"))) st->setYErrorMm(o.value(QStringLiteral("yErrorMm")).toDouble());
    if (o.contains(QStringLiteral("simplifyToleranceMm"))) st->setSimplifyToleranceMm(o.value(QStringLiteral("simplifyToleranceMm")).toDouble());
    if (o.contains(QStringLiteral("minSegmentMm"))) st->setMinSegmentMm(o.value(QStringLiteral("minSegmentMm")).toDouble());
    if (o.contains(QStringLiteral("collinearToleranceMm"))) st->setCollinearToleranceMm(o.value(QStringLiteral("collinearToleranceMm")).toDouble());
    if (o.contains(QStringLiteral("streamingPreset"))) st->setStreamingPreset(o.value(QStringLiteral("streamingPreset")).toString());
    if (o.contains(QStringLiteral("arcFitEnabled"))) st->setArcFitEnabled(o.value(QStringLiteral("arcFitEnabled")).toBool());
    if (o.contains(QStringLiteral("arcFitToleranceMm"))) st->setArcFitToleranceMm(o.value(QStringLiteral("arcFitToleranceMm")).toDouble());
    if (o.contains(QStringLiteral("grblJunctionDeviation"))) st->setGrblJunctionDeviation(o.value(QStringLiteral("grblJunctionDeviation")).toDouble());
    if (o.contains(QStringLiteral("grblAccelX"))) st->setGrblAccelX(o.value(QStringLiteral("grblAccelX")).toDouble());
    if (o.contains(QStringLiteral("grblAccelY"))) st->setGrblAccelY(o.value(QStringLiteral("grblAccelY")).toDouble());
    if (o.contains(QStringLiteral("penUpZ"))) st->setPenUpZ(o.value(QStringLiteral("penUpZ")).toDouble());
    if (o.contains(QStringLiteral("penDownZ"))) st->setPenDownZ(o.value(QStringLiteral("penDownZ")).toDouble());
    if (o.contains(QStringLiteral("previewDisplayScale"))) st->setPreviewDisplayScale(o.value(QStringLiteral("previewDisplayScale")).toDouble());
}
}

bool WriterProjectService::saveProject(const QString &path, WriterController *ctrl, QString *errorMessage) {
    if (!ctrl) {
        if (errorMessage) *errorMessage = QStringLiteral("Controller is null.");
        return false;
    }
    if (path.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("File path is empty.");
        return false;
    }
    QJsonObject root;
    root.insert(QStringLiteral("formatVersion"), kFormatVersion);
    root.insert(QStringLiteral("documentText"), ctrl->document()->text());
    root.insert(QStringLiteral("viewMode"), ctrl->viewMode());
    root.insert(QStringLiteral("fontFolderPath"), ctrl->fontFolderPath());
    root.insert(QStringLiteral("settings"), settingsToJson(ctrl->settings()));

    QJsonArray anchors;
    const QHash<int, QPointF> manual = ctrl->manualAnchors();
    for (auto it = manual.constBegin(); it != manual.constEnd(); ++it) {
        QJsonObject a;
        a.insert(QStringLiteral("docIndex"), it.key());
        a.insert(QStringLiteral("x"), it.value().x());
        a.insert(QStringLiteral("y"), it.value().y());
        anchors.append(a);
    }
    root.insert(QStringLiteral("manualAnchors"), anchors);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool WriterProjectService::loadProject(const QString &path, WriterController *ctrl, QString *errorMessage) {
    if (!ctrl) {
        if (errorMessage) *errorMessage = QStringLiteral("Controller is null.");
        return false;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        if (errorMessage) *errorMessage = QStringLiteral("Invalid project file.");
        return false;
    }
    const QJsonObject root = doc.object();
    const int fmt = root.value(QStringLiteral("formatVersion")).toInt(0);
    if (fmt < 1 || fmt > kFormatVersion) {
        if (errorMessage) *errorMessage = QStringLiteral("Unsupported project format version.");
        return false;
    }

    ctrl->beginProjectLoad();

    applySettingsFromJson(ctrl->settings(), root.value(QStringLiteral("settings")).toObject());

    const QString text = root.value(QStringLiteral("documentText")).toString();
    ctrl->document()->setText(text);

    QHash<int, QPointF> manual;
    const QJsonArray arr = root.value(QStringLiteral("manualAnchors")).toArray();
    for (const QJsonValue &v : arr) {
        const QJsonObject a = v.toObject();
        const int idx = a.value(QStringLiteral("docIndex")).toInt(-1);
        if (idx < 0 || idx >= text.size()) continue;
        const double x = a.value(QStringLiteral("x")).toDouble();
        const double y = a.value(QStringLiteral("y")).toDouble();
        manual.insert(idx, QPointF(x, y));
    }
    ctrl->setManualAnchors(manual);

    const QString vm = root.value(QStringLiteral("viewMode")).toString();
    if (vm == QStringLiteral("handwriting") || vm == QStringLiteral("typing")) ctrl->setViewMode(vm);

    const QString fontPath = root.value(QStringLiteral("fontFolderPath")).toString();
    ctrl->setFontFolderPathAndLoad(fontPath, true);
    ctrl->endProjectLoad();
    return true;
}
