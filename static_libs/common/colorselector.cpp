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
#include "ui_colorselector.h"

#include <QColorDialog>

#include "leakdetector.h"

ColorSelector::ColorSelector(QColor& color, const QColor& defaultColor, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ColorSelector)
    , m_color(color)
    , m_defaultColor(std::move(defaultColor))
{
    ui->setupUi(this);
    ui->frSelectColor->installEventFilter(this);
    ui->frSelectColor->setStyleSheet("QFrame { background: " + m_color.name(QColor::HexArgb) + " }");
}

ColorSelector::~ColorSelector()
{
    delete ui;
}

void ColorSelector::on_pbResetColor_clicked()
{
    m_color = m_defaultColor;
    ui->frSelectColor->setStyleSheet("QFrame { background: " + m_color.name(QColor::HexArgb) + " }");
}

bool ColorSelector::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->frSelectColor) {
        if (event->type() == QEvent::MouseButtonPress) {
            QColorDialog dialog(m_color);
            dialog.setOption(QColorDialog::ShowAlphaChannel, true);
            QColor color(m_color);
            connect(&dialog, &QColorDialog::currentColorChanged, [&color](const QColor& c) { color = c; });
            if (dialog.exec() && m_color != color) {
                m_color = color; //dialog.currentColor();
                ui->frSelectColor->setStyleSheet("QFrame { background: " + m_color.name(QColor::HexArgb) + " }");
            }
            return true;
        } else {
            return false;
        }
    } else {
        return QWidget::eventFilter(watched, event);
    }
}
