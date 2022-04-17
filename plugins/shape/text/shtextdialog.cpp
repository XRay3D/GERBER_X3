// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shtextdialog.h"
#include "shtext.h"
#include "ui_shtextdialog.h"

using namespace Shapes;

ShTextDialog::ShTextDialog(QVector<Text*> text, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ShTextDialog)
    , shapeText(text) {
    ui->setupUi(this);

    for (auto text : shapeText)
        text->save();

    ui->plainTextEdit->setStyleSheet("QPlainTextEdit { font-size: 32pt }");
    ui->cbxSide->addItems(QObject::tr("Top|Bottom").split('|'));

    ui->cbxFont->setFontFilters(
        QFontComboBox::ScalableFonts
        //        | QFontComboBox::NonScalableFonts
        | QFontComboBox::MonospacedFonts
        | QFontComboBox::ProportionalFonts);

    {
        QFont font;
        font.fromString(shapeText.first()->iData.font);
        ui->cbxFont->setCurrentFont(font);
        ui->plainTextEdit->setFont(font);
        ui->chbxBold->setChecked(font.bold());
        ui->chbxItalic->setChecked(font.italic());
    }

    ui->cbxSide->setCurrentIndex(static_cast<int>(shapeText.first()->iData.side));
    ui->plainTextEdit->setPlainText(shapeText.first()->iData.text);
    ui->dsbxAngle->setValue(shapeText.first()->iData.angle);
    ui->dsbxHeight->setValue(shapeText.first()->iData.height);
    ui->dsbxXY->setValue(shapeText.first()->iData.xy);

    switch (shapeText.first()->iData.handleAlign) {
    case Text::BotCenter:
        ui->rb_bc->setChecked(true);
        break;
    case Text::BotLeft:
        ui->rb_bl->setChecked(true);
        break;
    case Text::BotRight:
        ui->rb_br->setChecked(true);
        break;
    case Text::Center:
        ui->rb_c->setChecked(true);
        break;
    case Text::CenterLeft:
        ui->rb_lc->setChecked(true);
        break;
    case Text::CenterRight:
        ui->rb_rc->setChecked(true);
        break;
    case Text::TopCenter:
        ui->rb_tc->setChecked(true);
        break;
    case Text::TopLeft:
        ui->rb_tl->setChecked(true);
        break;
    case Text::TopRight:
        ui->rb_tr->setChecked(true);
        break;
    }

    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &ShTextDialog::updateText);
    connect(ui->dsbxAngle, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ShTextDialog::updateAngle);
    connect(ui->dsbxHeight, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ShTextDialog::updateHeight);
    connect(ui->dsbxXY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ShTextDialog::updateXY);
    connect(ui->cbxFont, qOverload<const QFont&>(&QFontComboBox::currentFontChanged), [this](const QFont&) { updateFont(); });
    connect(ui->cbxSide, qOverload<int>(&QComboBox::currentIndexChanged), this, &ShTextDialog::updateSide);

    connect(ui->rb_bc, &QRadioButton::toggled, this, &ShTextDialog::updateCenterAlign);
    connect(ui->rb_bl, &QRadioButton::toggled, this, &ShTextDialog::updateCenterAlign);
    connect(ui->rb_br, &QRadioButton::toggled, this, &ShTextDialog::updateCenterAlign);
    connect(ui->rb_c, &QRadioButton::toggled, this, &ShTextDialog::updateCenterAlign);
    connect(ui->rb_lc, &QRadioButton::toggled, this, &ShTextDialog::updateCenterAlign);
    connect(ui->rb_rc, &QRadioButton::toggled, this, &ShTextDialog::updateCenterAlign);
    connect(ui->rb_tc, &QRadioButton::toggled, this, &ShTextDialog::updateCenterAlign);
    connect(ui->rb_tl, &QRadioButton::toggled, this, &ShTextDialog::updateCenterAlign);
    connect(ui->rb_tr, &QRadioButton::toggled, this, &ShTextDialog::updateCenterAlign);

    connect(ui->chbxBold, &QCheckBox::toggled, this, &ShTextDialog::updateFont);
    connect(ui->chbxItalic, &QCheckBox::toggled, this, &ShTextDialog::updateFont);
}

ShTextDialog::~ShTextDialog() { delete ui; }

void ShTextDialog::updateText() {
    QString text_(ui->plainTextEdit->toPlainText());
    for (auto text : shapeText) {
        text->iData.text = text_;
        text->redraw();
    }
}

void ShTextDialog::updateFont() {
    QFont font(ui->cbxFont->currentFont());
    font.setBold(ui->chbxBold->isChecked());
    font.setItalic(ui->chbxItalic->isChecked());
    ui->plainTextEdit->setFont(font);
    QString strFont(font.toString());
    for (auto text : shapeText) {
        text->iData.font = strFont;
        text->redraw();
    }
}

void ShTextDialog::updateAngle() {
    for (auto text : shapeText) {
        text->iData.angle = ui->dsbxAngle->value();
        text->redraw();
    }
}

void ShTextDialog::updateHeight() {
    for (auto text : shapeText) {
        text->iData.height = ui->dsbxHeight->value();
        text->redraw();
    }
}

void ShTextDialog::updateXY() {
    for (auto text : shapeText) {
        text->iData.xy = ui->dsbxXY->value();
        text->redraw();
    }
}

void ShTextDialog::updateCenterAlign() {
    int handleAlign;
    if (ui->rb_bc->isChecked()) {
        handleAlign = Text::BotCenter;
    } else if (ui->rb_bl->isChecked()) {
        handleAlign = Text::BotLeft;
    } else if (ui->rb_br->isChecked()) {
        handleAlign = Text::BotRight;
    } else if (ui->rb_c->isChecked()) {
        handleAlign = Text::Center;
    } else if (ui->rb_lc->isChecked()) {
        handleAlign = Text::CenterLeft;
    } else if (ui->rb_rc->isChecked()) {
        handleAlign = Text::CenterRight;
    } else if (ui->rb_tc->isChecked()) {
        handleAlign = Text::TopCenter;
    } else if (ui->rb_tl->isChecked()) {
        handleAlign = Text::TopLeft;
    } else if (ui->rb_tr->isChecked()) {
        handleAlign = Text::TopRight;
    }
    for (auto text : shapeText) {
        text->iData.handleAlign = handleAlign;
        text->redraw();
    }
}

void ShTextDialog::updateSide() {
    for (auto text : shapeText) {
        text->iData.side = static_cast<Side>(ui->cbxSide->currentIndex());
        text->redraw();
    }
}

void ShTextDialog::accept() {
    shapeText.first()->ok();
    QDialog::accept();
}

void ShTextDialog::reject() {
    for (auto text : shapeText)
        text->restore();
    QDialog::reject();
}
