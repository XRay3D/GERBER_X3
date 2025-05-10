/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "colorselector.h"
#include "utils.h"

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
        if(dialog.exec() && color_ != color)
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
    : QWidget{parent}
    , color_(color)
    , defaultColor_(defaultColor) {
    if(objectName().isEmpty())
        setObjectName(u"ColorSelector"_s);
    auto horizontalLayout = new QHBoxLayout{this};
    horizontalLayout->setObjectName(u"horizontalLayout"_s);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    lineEdit = new QLineEdit{this};
    lineEdit->setObjectName(u"lineEdit"_s);
    lineEdit->setReadOnly(true);
    horizontalLayout->addWidget(lineEdit);

    pbSelectColor = new PushButton{color, this};
    pbSelectColor->setObjectName(u"pbSelectColor"_s);
    horizontalLayout->addWidget(pbSelectColor);

    pbResetColor = new QPushButton{tr("Reset"), this};
    pbResetColor->setObjectName(u"pbResetColor"_s);
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
