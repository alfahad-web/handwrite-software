#include "ProjectService.h"

#include "../core/EditorTypes.h"

#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ProjectService::ProjectService(QObject *parent) : QObject(parent) {}

bool ProjectService::saveProject(const QString &path, const EditorStore &store, QString *errorMessage) const {
    if (path.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("Project path is empty.");
        return false;
    }
    QJsonObject root;
    root.insert("formatVersion", 2);
    root.insert("strokePx", store.strokePx());
    root.insert("captureGapUm", store.captureGapUm());
    root.insert("zoom", store.zoom());
    root.insert("eraseRadiusPx", store.eraseRadiusPx());

    QJsonArray strokes;
    for (const Stroke &stroke : store.strokes()) {
        QJsonObject s;
        s.insert("id", stroke.id);
        s.insert("createdAt", QString::number(stroke.createdAt));
        QJsonArray points;
        for (const Stroke::StrokePoint &pt : stroke.points) {
            QJsonObject p;
            p.insert("x", pt.pos.x());
            p.insert("y", pt.pos.y());
            p.insert("erased", pt.erased);
            points.push_back(p);
        }
        s.insert("points", points);
        strokes.push_back(s);
    }
    root.insert("strokes", strokes);

    QJsonArray boxes;
    for (const SelectionBox &box : store.selectionBoxes()) {
        QJsonObject b;
        b.insert("id", box.id);
        b.insert("orderIndex", box.orderIndex);
        b.insert("x", box.rect.x);
        b.insert("y", box.rect.y);
        b.insert("width", box.rect.width);
        b.insert("height", box.rect.height);
        b.insert("assigned", box.assigned);
        b.insert("assignedAscii", box.assignedAscii);
        b.insert("fileStem", box.fileStem);
        b.insert("joinMode", joinModeToString(box.joinMode));
        b.insert("hasManualAnchor", box.hasManualAnchor);
        b.insert("manualAnchorRx", box.manualAnchorRx);
        b.insert("manualAnchorRy", box.manualAnchorRy);
        b.insert("anchorX", box.anchorX);
        b.insert("anchorY", box.anchorY);
        boxes.push_back(b);
    }
    root.insert("selectionBoxes", boxes);
    root.insert("selectedSelectionId", store.selectedSelectionId());
    QJsonObject selectionErasedPoints;
    for (auto it = store.selectionErasedPointKeys().cbegin(); it != store.selectionErasedPointKeys().cend(); ++it) {
        QJsonArray keys;
        for (const QString &key : it.value()) keys.push_back(key);
        selectionErasedPoints.insert(it.key(), keys);
    }
    root.insert("selectionErasedPoints", selectionErasedPoints);

    QJsonObject stems;
    for (auto it = store.specialCharStemMap().cbegin(); it != store.specialCharStemMap().cend(); ++it) {
        stems.insert(QString::number(it.key()), it.value());
    }
    root.insert("specialCharStemMap", stems);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ProjectService::loadProject(const QString &path, EditorStore *store, QString *errorMessage) const {
    if (!store) {
        if (errorMessage) *errorMessage = QStringLiteral("Store is null.");
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
    const int fmt = root.value("formatVersion").toInt(0);
    if (fmt != 1 && fmt != 2) {
        if (errorMessage) *errorMessage = QStringLiteral("Unsupported project version.");
        return false;
    }

    store->clearAll();
    store->setStrokePx(root.value("strokePx").toInt(store->strokePx()));
    store->setCaptureGapUm(root.value("captureGapUm").toInt(store->captureGapUm()));
    store->setZoom(root.value("zoom").toInt(store->zoom()));
    store->setEraseRadiusPx(root.value("eraseRadiusPx").toInt(store->eraseRadiusPx()));

    // Recreate strokes
    QVector<Stroke> loadedStrokes;
    const QJsonArray strokes = root.value("strokes").toArray();
    for (const QJsonValue &sv : strokes) {
        const QJsonObject s = sv.toObject();
        const QJsonArray points = s.value("points").toArray();
        if (points.isEmpty()) continue;
        Stroke stroke;
        stroke.id = s.value("id").toString();
        stroke.createdAt = s.value("createdAt").toString().toLongLong();
        stroke.points.reserve(points.size());
        for (const QJsonValue &pv : points) {
            const QJsonObject p = pv.toObject();
            Stroke::StrokePoint point;
            point.pos = QPointF(p.value("x").toDouble(), p.value("y").toDouble());
            point.erased = p.value("erased").toBool(false);
            stroke.points.push_back(point);
        }
        loadedStrokes.push_back(stroke);
    }
    store->setStrokes(loadedStrokes);

    // Recreate boxes
    QVector<SelectionBox> loadedBoxes;
    const QJsonArray boxes = root.value("selectionBoxes").toArray();
    for (const QJsonValue &bv : boxes) {
        const QJsonObject b = bv.toObject();
        SelectionBox box;
        box.id = b.value("id").toString();
        box.orderIndex = b.value("orderIndex").toInt(0);
        box.rect = SelectionRect{
            b.value("x").toDouble(),
            b.value("y").toDouble(),
            b.value("width").toDouble(),
            b.value("height").toDouble()};
        box.assigned = b.value("assigned").toBool(false);
        box.assignedAscii = b.value("assignedAscii").toInt(-1);
        box.fileStem = b.value("fileStem").toString();
        box.joinMode = joinModeFromString(b.value("joinMode").toString());
        box.hasManualAnchor = b.value("hasManualAnchor").toBool(false);
        box.manualAnchorRx = b.value("manualAnchorRx").toDouble(0.0);
        box.manualAnchorRy = b.value("manualAnchorRy").toDouble(0.0);
        box.anchorX = b.value("anchorX").toDouble(0.0);
        box.anchorY = b.value("anchorY").toDouble(0.0);
        loadedBoxes.push_back(box);
    }
    store->setSelectionBoxes(loadedBoxes, root.value("selectedSelectionId").toString());
    QHash<QString, QSet<QString>> selectionErasedPoints;
    const QJsonObject selectionErasedObj = root.value("selectionErasedPoints").toObject();
    for (auto it = selectionErasedObj.begin(); it != selectionErasedObj.end(); ++it) {
        const QJsonArray keys = it.value().toArray();
        QSet<QString> keySet;
        for (const QJsonValue &kv : keys) {
            const QString key = kv.toString();
            if (!key.isEmpty()) keySet.insert(key);
        }
        if (!keySet.isEmpty()) selectionErasedPoints.insert(it.key(), keySet);
    }
    store->setSelectionErasedPointKeys(selectionErasedPoints);

    QHash<int, QString> stemMap;
    const QJsonObject stemObj = root.value("specialCharStemMap").toObject();
    for (auto it = stemObj.begin(); it != stemObj.end(); ++it) stemMap.insert(it.key().toInt(), it.value().toString());
    store->setSpecialCharStemMap(stemMap);
    store->setProjectFilePath(path);
    store->markSaved();
    return true;
}
