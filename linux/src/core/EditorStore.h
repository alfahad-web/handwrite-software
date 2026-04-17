#pragma once

#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>

#include "EditorTypes.h"

class EditorStore : public QObject {
    Q_OBJECT
    Q_PROPERTY(int strokePx READ strokePx WRITE setStrokePx NOTIFY strokePxChanged)
    Q_PROPERTY(int captureGapUm READ captureGapUm WRITE setCaptureGapUm NOTIFY captureGapUmChanged)
    Q_PROPERTY(int zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QString toolMode READ toolMode WRITE setToolMode NOTIFY toolModeChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(bool isDirty READ isDirty NOTIFY isDirtyChanged)
    Q_PROPERTY(QString openFilePath READ openFilePath NOTIFY openFilePathChanged)
    Q_PROPERTY(QString openFileName READ openFileName NOTIFY openFileNameChanged)

public:
    explicit EditorStore(QObject *parent = nullptr);

    int strokePx() const;
    int captureGapUm() const;
    int zoom() const;
    QString toolMode() const;
    bool hasSelection() const;
    bool isDirty() const;
    QString openFilePath() const;
    QString openFileName() const;

    const QVector<Stroke> &strokes() const;
    QString currentStrokeId() const;
    ToolMode toolModeValue() const;
    const SelectionRect *selectionRect() const;
    const SelectionRect *selectionDraftRect() const;
    const ResizeDragState *selectionResizeState() const;

    Q_INVOKABLE void setStrokePx(int value);
    Q_INVOKABLE void setCaptureGapUm(int value);
    Q_INVOKABLE void setZoom(int value);
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void setToolMode(const QString &mode);
    Q_INVOKABLE void toggleToolMode();

    void startStroke(const QPointF &point);
    void replaceActiveStrokePoints(const QVector<QPointF> &points);
    void endStroke();
    void setSelectionDraftRect(const SelectionRect *rect);
    void commitSelectionDraftRect();
    void clearSelection();
    void setSelectionRect(const SelectionRect *rect);
    void setSelectionResizeState(const ResizeDragState *state);

    void setOpenFile(const QString &path);
    void closeFile();
    void markSaved();
    void markDirty();

signals:
    void strokePxChanged();
    void captureGapUmChanged();
    void zoomChanged();
    void toolModeChanged();
    void selectionChanged();
    void strokesChanged();
    void isDirtyChanged();
    void openFilePathChanged();
    void openFileNameChanged();

private:
    static SelectionRect normalizeRect(const SelectionRect &rect, bool *ok = nullptr);
    static int clampInt(int value, int min, int max);
    static QString makeStrokeId();
    int findStrokeIndexById(const QString &id) const;

    QVector<Stroke> m_strokes;
    QString m_currentStrokeId;
    ToolMode m_toolMode;
    int m_strokePx;
    int m_captureGapUm;
    int m_zoom;
    bool m_hasSelectionRect;
    SelectionRect m_selectionRect;
    bool m_hasSelectionDraftRect;
    SelectionRect m_selectionDraftRect;
    bool m_hasResizeState;
    ResizeDragState m_resizeState;
    QString m_openFilePath;
    QString m_openFileName;
    bool m_isDirty;
};
