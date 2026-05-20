#pragma once

#include <QObject>
#include <QString>

class AppSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(double feedRateCmPerS READ feedRateCmPerS WRITE setFeedRateCmPerS NOTIFY feedRateCmPerSChanged)
    Q_PROPERTY(double pageWidthCm READ pageWidthCm WRITE setPageWidthCm NOTIFY pageWidthCmChanged)
    Q_PROPERTY(double pageHeightCm READ pageHeightCm WRITE setPageHeightCm NOTIFY pageHeightCmChanged)
    Q_PROPERTY(double leftMarginCm READ leftMarginCm WRITE setLeftMarginCm NOTIFY leftMarginCmChanged)
    Q_PROPERTY(double rightMarginCm READ rightMarginCm WRITE setRightMarginCm NOTIFY rightMarginCmChanged)
    Q_PROPERTY(double verticalGapCm READ verticalGapCm WRITE setVerticalGapCm NOTIFY verticalGapCmChanged)
    Q_PROPERTY(double hxCm READ hxCm WRITE setHxCm NOTIFY hxCmChanged)
    Q_PROPERTY(double hyCm READ hyCm WRITE setHyCm NOTIFY hyCmChanged)
    Q_PROPERTY(double lineHeightCm READ lineHeightCm WRITE setLineHeightCm NOTIFY lineHeightCmChanged)
    Q_PROPERTY(double fontUnitToCm READ fontUnitToCm WRITE setFontUnitToCm NOTIFY fontUnitToCmChanged)
    Q_PROPERTY(double joinDistMm READ joinDistMm WRITE setJoinDistMm NOTIFY joinDistMmChanged)
    Q_PROPERTY(double backlashYStartMm READ backlashYStartMm WRITE setBacklashYStartMm NOTIFY backlashYStartMmChanged)
    Q_PROPERTY(double backlashYEndMm READ backlashYEndMm WRITE setBacklashYEndMm NOTIFY backlashYEndMmChanged)
    Q_PROPERTY(double xErrorNearMm READ xErrorNearMm WRITE setXErrorNearMm NOTIFY xErrorNearMmChanged)
    Q_PROPERTY(double xErrorMm READ xErrorMm WRITE setXErrorMm NOTIFY xErrorMmChanged)
    Q_PROPERTY(double yErrorNearMm READ yErrorNearMm WRITE setYErrorNearMm NOTIFY yErrorNearMmChanged)
    Q_PROPERTY(double yErrorMm READ yErrorMm WRITE setYErrorMm NOTIFY yErrorMmChanged)
    Q_PROPERTY(double simplifyToleranceMm READ simplifyToleranceMm WRITE setSimplifyToleranceMm NOTIFY simplifyToleranceMmChanged)
    Q_PROPERTY(double minSegmentMm READ minSegmentMm WRITE setMinSegmentMm NOTIFY minSegmentMmChanged)
    Q_PROPERTY(double collinearToleranceMm READ collinearToleranceMm WRITE setCollinearToleranceMm NOTIFY collinearToleranceMmChanged)
    Q_PROPERTY(QString streamingPreset READ streamingPreset WRITE setStreamingPreset NOTIFY streamingPresetChanged)
    Q_PROPERTY(bool arcFitEnabled READ arcFitEnabled WRITE setArcFitEnabled NOTIFY arcFitEnabledChanged)
    Q_PROPERTY(double arcFitToleranceMm READ arcFitToleranceMm WRITE setArcFitToleranceMm NOTIFY arcFitToleranceMmChanged)
    Q_PROPERTY(double grblJunctionDeviation READ grblJunctionDeviation WRITE setGrblJunctionDeviation NOTIFY grblJunctionDeviationChanged)
    Q_PROPERTY(double grblAccelX READ grblAccelX WRITE setGrblAccelX NOTIFY grblAccelXChanged)
    Q_PROPERTY(double grblAccelY READ grblAccelY WRITE setGrblAccelY NOTIFY grblAccelYChanged)
    Q_PROPERTY(bool servoSnapMode READ servoSnapMode WRITE setServoSnapMode NOTIFY servoSnapModeChanged)
    Q_PROPERTY(double servoUpS READ servoUpS WRITE setServoUpS NOTIFY servoUpSChanged)
    Q_PROPERTY(double servoDownS READ servoDownS WRITE setServoDownS NOTIFY servoDownSChanged)
    Q_PROPERTY(double penUpZ READ penUpZ WRITE setPenUpZ NOTIFY penUpZChanged)
    Q_PROPERTY(double penDownZ READ penDownZ WRITE setPenDownZ NOTIFY penDownZChanged)
    Q_PROPERTY(double feedRateMmPerMin READ feedRateMmPerMin NOTIFY feedRateMmPerMinChanged)
    Q_PROPERTY(double previewDisplayScale READ previewDisplayScale WRITE setPreviewDisplayScale NOTIFY previewDisplayScaleChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);

    double feedRateCmPerS() const { return m_feedRateCmPerS; }
    void setFeedRateCmPerS(double v);

    double pageWidthCm() const { return m_pageWidthCm; }
    void setPageWidthCm(double v);

    double pageHeightCm() const { return m_pageHeightCm; }
    void setPageHeightCm(double v);

    double leftMarginCm() const { return m_leftMarginCm; }
    void setLeftMarginCm(double v);

    double rightMarginCm() const { return m_rightMarginCm; }
    void setRightMarginCm(double v);

    double verticalGapCm() const { return m_verticalGapCm; }
    void setVerticalGapCm(double v);

    double hxCm() const { return m_hxCm; }
    void setHxCm(double v);

    double hyCm() const { return m_hyCm; }
    void setHyCm(double v);

    double lineHeightCm() const { return m_lineHeightCm; }
    void setLineHeightCm(double v);

    double fontUnitToCm() const { return m_fontUnitToCm; }
    void setFontUnitToCm(double v);

    double joinDistMm() const { return m_joinDistMm; }
    void setJoinDistMm(double v);

    double backlashYStartMm() const { return m_backlashYStartMm; }
    void setBacklashYStartMm(double v);

    double backlashYEndMm() const { return m_backlashYEndMm; }
    void setBacklashYEndMm(double v);

    double xErrorNearMm() const { return m_xErrorNearMm; }
    void setXErrorNearMm(double v);

    double xErrorMm() const { return m_xErrorMm; }
    void setXErrorMm(double v);

    double yErrorNearMm() const { return m_yErrorNearMm; }
    void setYErrorNearMm(double v);

    double yErrorMm() const { return m_yErrorMm; }
    void setYErrorMm(double v);

    double simplifyToleranceMm() const { return m_simplifyToleranceMm; }
    void setSimplifyToleranceMm(double v);

    double minSegmentMm() const { return m_minSegmentMm; }
    void setMinSegmentMm(double v);

    double collinearToleranceMm() const { return m_collinearToleranceMm; }
    void setCollinearToleranceMm(double v);

    QString streamingPreset() const { return m_streamingPreset; }
    void setStreamingPreset(const QString &v);

    bool arcFitEnabled() const { return m_arcFitEnabled; }
    void setArcFitEnabled(bool v);

    double arcFitToleranceMm() const { return m_arcFitToleranceMm; }
    void setArcFitToleranceMm(double v);

    double grblJunctionDeviation() const { return m_grblJunctionDeviation; }
    void setGrblJunctionDeviation(double v);

    double grblAccelX() const { return m_grblAccelX; }
    void setGrblAccelX(double v);

    double grblAccelY() const { return m_grblAccelY; }
    void setGrblAccelY(double v);

    bool servoSnapMode() const { return m_servoSnapMode; }
    void setServoSnapMode(bool v);

    double servoUpS() const { return m_servoUpS; }
    void setServoUpS(double v);

    double servoDownS() const { return m_servoDownS; }
    void setServoDownS(double v);

    double penUpZ() const { return m_penUpZ; }
    void setPenUpZ(double v);

    double penDownZ() const { return m_penDownZ; }
    void setPenDownZ(double v);

    double feedRateMmPerMin() const { return m_feedRateCmPerS * 600.0; }

    double previewDisplayScale() const { return m_previewDisplayScale; }
    void setPreviewDisplayScale(double v);

    Q_INVOKABLE void load();
    Q_INVOKABLE void save();

signals:
    void feedRateCmPerSChanged();
    void pageWidthCmChanged();
    void pageHeightCmChanged();
    void leftMarginCmChanged();
    void rightMarginCmChanged();
    void verticalGapCmChanged();
    void hxCmChanged();
    void hyCmChanged();
    void lineHeightCmChanged();
    void fontUnitToCmChanged();
    void joinDistMmChanged();
    void backlashYStartMmChanged();
    void backlashYEndMmChanged();
    void xErrorNearMmChanged();
    void xErrorMmChanged();
    void yErrorNearMmChanged();
    void yErrorMmChanged();
    void simplifyToleranceMmChanged();
    void minSegmentMmChanged();
    void collinearToleranceMmChanged();
    void streamingPresetChanged();
    void arcFitEnabledChanged();
    void arcFitToleranceMmChanged();
    void grblJunctionDeviationChanged();
    void grblAccelXChanged();
    void grblAccelYChanged();
    void servoSnapModeChanged();
    void servoUpSChanged();
    void servoDownSChanged();
    void penUpZChanged();
    void penDownZChanged();
    void feedRateMmPerMinChanged();
    void previewDisplayScaleChanged();
    void anyChanged();
    void aboutToChange();

private:
    double m_feedRateCmPerS = 2.0;
    double m_pageWidthCm = 21.0;
    double m_pageHeightCm = 29.7;
    double m_leftMarginCm = 1.5;
    double m_rightMarginCm = 1.5;
    double m_verticalGapCm = 0.5;
    double m_hxCm = 0.2;
    double m_hyCm = 0.5;
    double m_lineHeightCm = 0.45;
    double m_fontUnitToCm = 0.0001;
    double m_joinDistMm = 0.0;
    double m_backlashYStartMm = 0.0;
    double m_backlashYEndMm = 297.0;
    double m_xErrorNearMm = 0.0;
    double m_xErrorMm = 0.0;
    double m_yErrorNearMm = 0.0;
    double m_yErrorMm = 0.0;
    double m_simplifyToleranceMm = 0.0;
    double m_minSegmentMm = 0.05;
    double m_collinearToleranceMm = 0.02;
    QString m_streamingPreset = QStringLiteral("balanced");
    bool m_arcFitEnabled = false;
    double m_arcFitToleranceMm = 0.05;
    double m_grblJunctionDeviation = 0.03;
    double m_grblAccelX = 300.0;
    double m_grblAccelY = 300.0;
    bool m_servoSnapMode = false;
    double m_servoUpS = 0.0;
    double m_servoDownS = 1000.0;
    double m_penUpZ = 30.0;
    double m_penDownZ = -5.0;
    double m_previewDisplayScale = 1.0;
};
