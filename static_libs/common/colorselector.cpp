// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "colorselector.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>

class PushButton : public QPushButton {
    //    Q_OBJECT
    QColor& color_;
    void selectColor() {
        QColorDialog dialog(color_);
        dialog.setOption(QColorDialog::ShowAlphaChannel, true);
        QColor color(color_);
        connect(&dialog, &QColorDialog::currentColorChanged, [&color](const QColor& c) { color = c; });
        if (dialog.exec() && color_ != color)
            color_ = color;
        //        setText("ARGB " + color_.name(QColor::HexArgb).toUpper());
    }

public:
    PushButton(QColor& color, QWidget* parent = nullptr)
        : QPushButton("", parent)
        , color_(color) {
        connect(this, &QPushButton::clicked, this, &PushButton::selectColor);
        //        setText("ARGB " + color_.name(QColor::HexArgb).toUpper());
    }
    virtual ~PushButton() { }

protected:
    void paintEvent(QPaintEvent* event) override {
        QPushButton::paintEvent(event);
        QPainter p(this);
        p.setPen(Qt::NoPen);

        //        p.setBrush(Qt::white);
        //        p.drawRect(rect() + QMargins(-3, -3, -3, -3));
        QLinearGradient gr(rect().topRight(), rect().bottomLeft());
        gr.setColorAt(0.1, Qt::black);
        gr.setColorAt(0.9, Qt::white);
        p.setBrush(gr);
        p.drawRect(rect() + QMargins(-3, -3, -3, -3));
        p.setBrush(color_);
        p.drawRect(rect() + QMargins(-3, -3, -3, -3));
        //        p.setCompositionMode(QPainter::CompositionMode_Xor);
        //        p.setPen(Qt::NoPen);
        //        p.setBrush(Qt::black);
        //        QPainterPath path;
        //        path.addText(rect().bottomLeft(), font() /*Qt::AlignCenter*/, text());
        //        p.drawPolygon(path.toFillPolygon());
        p.setPen(Qt::black);
        p.drawText(rect(), Qt::AlignCenter, text());
    }
};

ColorSelector::ColorSelector(QColor& color, const QColor& defaultColor, QWidget* parent)
    : QWidget(parent)
    , color_(color)
    , defaultColor_(defaultColor) {
    if (objectName().isEmpty())
        setObjectName(QString::fromUtf8("ColorSelector"));
    auto horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    lineEdit = new QLineEdit(this);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    lineEdit->setReadOnly(true);
    horizontalLayout->addWidget(lineEdit);

    pbSelectColor = new PushButton(color, this);
    pbSelectColor->setObjectName(QString::fromUtf8("pbSelectColor"));
    horizontalLayout->addWidget(pbSelectColor);

    pbResetColor = new QPushButton(tr("Reset"), this);
    pbResetColor->setObjectName(QString::fromUtf8("pbResetColor"));
    horizontalLayout->addWidget(pbResetColor);
    horizontalLayout->setStretch(1, 1);
    horizontalLayout->setStretch(2, 1);
    horizontalLayout->setStretch(3, 0);
    connect(pbResetColor, &QPushButton::clicked, this, &ColorSelector::resetColor);
    connect(pbResetColor, &QPushButton::clicked, this, &ColorSelector::updateName);
    connect(pbSelectColor, &QPushButton::clicked, this, &ColorSelector::updateName);
    updateName();
}

ColorSelector::~ColorSelector() { }

void ColorSelector::resetColor() {
    color_ = defaultColor_;
    pbSelectColor->update();
    updateName();
}

void ColorSelector::updateName() { lineEdit->setText(color_.name(QColor::HexArgb).toUpper()); }

#include "moc_colorselector.cpp"
