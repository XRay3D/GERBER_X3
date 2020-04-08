#pragma once
#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H

#include <QWidget>

namespace Ui {
class ColorSelector;
}

class ColorSelector : public QWidget {
    Q_OBJECT

public:
    explicit ColorSelector(QColor& color, const QColor& defaultColor, QWidget* parent = nullptr);
    ~ColorSelector() override;

private slots:
    void on_pbResetColor_clicked();

private:
    Ui::ColorSelector* ui;
    QColor& m_color;
    const QColor m_defaultColor;

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void setColor(QColor* color);
};

#endif // COLORSELECTOR_H
