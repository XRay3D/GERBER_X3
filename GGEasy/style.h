// #pragma once

// class Style : public QProxyStyle {
//     Q_OBJECT

// public:
//     explicit Style();
//     ~Style();
//     void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const override;
//     void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
//     void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget = nullptr) const override;
//     void drawItemText(QPainter* painter, const QRect& rect, int flags, const QPalette& pal, bool enabled, const QString& text, QPalette::ColorRole textRole = QPalette::NoRole) const override;
//     void drawItemPixmap(QPainter* painter, const QRect& rect, int alignment, const QPixmap& pixmap) const override;
// };

// class MyStyle : public QCommonStyle {
//     Q_OBJECT
// public:
//     explicit MyStyle();

//    void drawControl(ControlElement element, const QStyleOption* opt, QPainter* p, const QWidget* w) const;
//    void drawItemText(QPainter* painter, const QRect& rect, int flags, const QPalette& pal, bool enabled, const QString& text, QPalette::ColorRole textRole) const;
//    void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const;

// signals:

// public slots:
// };
