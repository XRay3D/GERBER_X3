// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "colorselector.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>

#include "leakdetector.h"

class PushButton : public QPushButton {
    //    Q_OBJECT
    QColor& m_color;
    void selectColor()
    {
        QColorDialog dialog(m_color);
        dialog.setOption(QColorDialog::ShowAlphaChannel, true);
        QColor color(m_color);
        connect(&dialog, &QColorDialog::currentColorChanged, [&color](const QColor& c) { color = c; });
        if (dialog.exec() && m_color != color)
            m_color = color;
        //        setText("ARGB " + m_color.name(QColor::HexArgb).toUpper());
    }

public:
    PushButton(QColor& color, QWidget* parent = nullptr)
        : QPushButton("", parent)
        , m_color(color)
    {
        connect(this, &QPushButton::clicked, this, &PushButton::selectColor);
        //        setText("ARGB " + m_color.name(QColor::HexArgb).toUpper());
    }
    virtual ~PushButton() { }

protected:
    void paintEvent(QPaintEvent* event) override
    {
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
        p.setBrush(m_color);
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
    , m_color(color)
    , m_defaultColor(std::move(defaultColor))
{
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

    connect(pbResetColor, &QPushButton::clicked, this, &ColorSelector::resetColor);
    connect(pbResetColor, &QPushButton::clicked, this, &ColorSelector::updateName);
    connect(pbSelectColor, &QPushButton::clicked, this, &ColorSelector::updateName);
    updateName();
}

ColorSelector::~ColorSelector() { }

void ColorSelector::resetColor()
{
    m_color = m_defaultColor;
    pbSelectColor->update();
    updateName();
}

void ColorSelector::updateName() { lineEdit->setText(m_color.name(QColor::HexArgb).toUpper()); }
