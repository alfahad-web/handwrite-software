#pragma once

#include <QObject>
#include <QHash>
#include <QPointF>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVector>

#include "EditorTypes.h"

class EditorStore : public QObject {
    Q_OBJECT
    Q_PROPERTY(int strokePx READ strokePx WRITE setStrokePx NOTIFY strokePxChanged)
    Q_PROPERTY(int captureGapUm READ captureGapUm WRITE setCaptureGapUm NOTIFY captureGapUmChanged)
    Q_PROPERTY(int guideLineGapPx READ guideLineGapPx WRITE setGuideLineGapPx NOTIFY guideLineGapPxChanged)
    Q_PROPERTY(int zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QString toolMode READ toolMode WRITE setToolMode NOTIFY toolModeChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(bool hasSelectedSelection READ hasSelectedSelection NOTIFY selectionChanged)
    Q_PROPERTY(int eraseRadiusPx READ eraseRadiusPx WRITE setEraseRadiusPx NOTIFY eraseRadiusPxChanged)
    Q_PROPERTY(bool drawStrokeEraseActive READ drawStrokeEraseActive WRITE setDrawStrokeEraseActive NOTIFY drawStrokeEraseActiveChanged)
    Q_PROPERTY(bool isDirty READ isDirty NOTIFY isDirtyChanged)
    Q_PROPERTY(QString projectFilePath READ projectFilePath NOTIFY projectFilePathChanged)
    Q_PROPERTY(QString projectFileName READ projectFileName NOTIFY projectFileNameChanged)

public:
    explicit EditorStore(QObject *parent = nullptr);

    int strokePx() const;
    int captureGapUm() const;
    int guideLineGapPx() const;
    int zoom() const;
    QString toolMode() const;
    bool hasSelection() const;
    bool hasSelectedSelection() const;
    int eraseRadiusPx() const;
    bool drawStrokeEraseActive() const;
    bool isDirty() const;
    QString projectFilePath() const;
    QString projectFileName() const;
    QString selectedSelectionId() const;

    const QVector<Stroke> &strokes() const;
    QString currentStrokeId() const;
    ToolMode toolModeValue() const;
    const QVector<SelectionBox> &selectionBoxes() const;
    const SelectionBox *selectedSelection() const;
    const SelectionBox *selectionById(const QString &id) const;
    SelectionBox *selectionByIdMutable(const QString &id);
    const SelectionRect *selectionDraftRect() const;
    const ResizeDragState *selectionResizeState() const;

    Q_INVOKABLE void setStrokePx(int value);
    Q_INVOKABLE void setCaptureGapUm(int value);
    Q_INVOKABLE void setGuideLineGapPx(int value);
    Q_INVOKABLE void setZoom(int value);
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void setToolMode(const QString &mode);
    Q_INVOKABLE void toggleToolMode();
    Q_INVOKABLE void setEraseRadiusPx(int value);
    Q_INVOKABLE void setDrawStrokeEraseActive(bool active);
    Q_INVOKABLE bool deleteSelectedSelection();

    void startStroke(const QPointF &point);
    void replaceActiveStrokePoints(const QVector<QPointF> &points);
    void endStroke();
    void setSelectionDraftRect(const SelectionRect *rect);
    QString commitSelectionDraftRect();
    void clearSelectionDraft();
    void setSelectionRect(const QString &selectionId, const SelectionRect *rect);
    void setSelectedSelectionId(const QString &selectionId);
    bool setSelectionAnchorPoint(const QString &selectionId, const QPointF &point);
    void setSelectionResizeState(const ResizeDragState *state);
    bool erasePointsInSelectedSelection(const QPointF &center, qreal radiusPx);
    bool erasePointsInSelectedSelectionPath(const QVector<QPointF> &centers, qreal radiusPx);
    bool removeStrokePointsNear(const QPointF &center, qreal radiusPx);
    bool removeStrokePointsNearPath(const QVector<QPointF> &centers, qreal radiusPx);
    bool isPointErasedInSelection(const QString &selectionId, const QString &strokeId, int pointIndex) const;

    void setProjectFilePath(const QString &path);
    void clearProjectFilePath();
    void clearAll();
    QString fileStemForAscii(int asciiCode);
    Q_INVOKABLE QVariantList selectionBoxesModel() const;

    const QHash<int, QString> &specialCharStemMap() const;
    const QHash<QString, QSet<QString>> &selectionErasedPointKeys() const;
    const QSet<QString> &highlightedSelectionIds() const;
    void setSpecialCharStemMap(const QHash<int, QString> &map);
    void setSelectionErasedPointKeys(const QHash<QString, QSet<QString>> &map);
    void setHighlightedSelectionIds(const QSet<QString> &ids);
    void clearHighlightedSelectionIds();
    void setStrokes(const QVector<Stroke> &strokes);
    void setSelectionBoxes(const QVector<SelectionBox> &boxes, const QString &selectedId);
    void markSaved();
    void markDirty();

    void recomputeSelectionAnchors();
    QPointF draftAnchorBoard() const;

signals:
    void strokePxChanged();
    void captureGapUmChanged();
    void guideLineGapPxChanged();
    void zoomChanged();
    void toolModeChanged();
    void selectionChanged();
    void strokesChanged();
    void eraseRadiusPxChanged();
    void drawStrokeEraseActiveChanged();
    void isDirtyChanged();
    void projectFilePathChanged();
    void projectFileNameChanged();

private:
    struct PointRef {
        int strokeIndex = -1;
        int pointIndex = -1;
    };

    static SelectionRect normalizeRect(const SelectionRect &rect, bool *ok = nullptr);
    static int clampInt(int value, int min, int max);
    static QString makeStrokeId();
    static QString makeSelectionId();
    static QString makeSpecialStem();
    static QString makePointKey(const QString &strokeId, int pointIndex);
    static qint64 makeCellKey(int cellX, int cellY);
    static int parsePointIndexFromKey(const QString &key);
    void clearPointSpatialIndex();
    void rebuildPointSpatialIndex();
    void ensurePointSpatialIndex();
    bool addSelectionErasedPoint(const QString &selectionId, const QString &strokeId, int pointIndex);
    int findStrokeIndexById(const QString &id) const;
    int findSelectionIndexById(const QString &id) const;

    QVector<Stroke> m_strokes;
    QString m_currentStrokeId;
    ToolMode m_toolMode;
    int m_strokePx;
    int m_captureGapUm;
    int m_guideLineGapPx;
    int m_zoom;
    int m_eraseRadiusPx;
    bool m_drawStrokeEraseActive = false;
    QVector<SelectionBox> m_selectionBoxes;
    QString m_selectedSelectionId;
    bool m_hasSelectionDraftRect;
    SelectionRect m_selectionDraftRect;
    bool m_hasResizeState;
    ResizeDragState m_resizeState;
    int m_nextSelectionOrder;
    QString m_projectFilePath;
    QString m_projectFileName;
    QHash<int, QString> m_specialCharStemMap;
    QHash<QString, QSet<QString>> m_selectionErasedPointKeys;
    QHash<QString, QHash<QString, QSet<int>>> m_selectionErasedPointIndex;
    QHash<qint64, QVector<PointRef>> m_pointSpatialIndex;
    bool m_pointSpatialIndexDirty = true;
    QSet<QString> m_highlightedSelectionIds;
    bool m_isDirty;
};
