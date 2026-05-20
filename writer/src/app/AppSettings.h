#pragma once

#include <QObject>

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
    Q_PROPERTY(double xErrorMm READ xErrorMm WRITE setXErrorMm NOTIFY xErrorMmChanged)
    Q_PROPERTY(double yErrorMm READ yErrorMm WRITE setYErrorMm NOTIFY yErrorMmChanged)
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

    double xErrorMm() const { return m_xErrorMm; }
    void setXErrorMm(double v);

    double yErrorMm() const { return m_yErrorMm; }
    void setYErrorMm(double v);

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
    void xErrorMmChanged();
    void yErrorMmChanged();
    void penUpZChanged();
    void penDownZChanged();
    void feedRateMmPerMinChanged();
    void previewDisplayScaleChanged();
    void anyChanged();

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
    double m_xErrorMm = 0.0;
    double m_yErrorMm = 0.0;
    double m_penUpZ = 30.0;
    double m_penDownZ = -5.0;
    double m_previewDisplayScale = 1.0;
};
