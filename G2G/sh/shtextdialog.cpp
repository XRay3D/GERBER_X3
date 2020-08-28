#include "shtextdialog.h"
#include "shtext.h"
#include "ui_shtextdialog.h"

using namespace Shapes;

ShTextDialog::ShTextDialog(Shapes::Text* text, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ShTextDialog)
    , pText(text)
{
    ui->setupUi(this);

    text_ = pText->text;
    font_ = pText->font;
    angle_ = pText->angle;
    height_ = pText->height;
    centerAlign_ = pText->centerAlign;

    ui->textEdit->setPlainText(pText->text);
    QFont f;
    f.fromString(pText->font);
    ui->fontComboBox->setCurrentFont(f);
    ui->dsbxAngle->setValue(pText->angle);
    ui->dsbxHeight->setValue(pText->height);

    switch (pText->centerAlign) {
    case 1:
        ui->rb_bc->setChecked(true);
        break;
    case 2:
        ui->rb_bl->setChecked(true);
        break;
    case 3:
        ui->rb_br->setChecked(true);
        break;
    case 4:
        ui->rb_c->setChecked(true);
        break;
    case 5:
        ui->rb_lc->setChecked(true);
        break;
    case 6:
        ui->rb_rc->setChecked(true);
        break;
    case 7:
        ui->rb_tc->setChecked(true);
        break;
    case 8:
        ui->rb_tl->setChecked(true);
        break;
    case 9:
        ui->rb_tr->setChecked(true);
        break;
    }

    connect(ui->textEdit, &QTextEdit::textChanged,
        this, &ShTextDialog::updateText);
    connect(ui->dsbxAngle, qOverload<double>(&QDoubleSpinBox::valueChanged),
        this, &ShTextDialog::updateText);
    connect(ui->dsbxHeight, qOverload<double>(&QDoubleSpinBox::valueChanged),
        this, &ShTextDialog::updateText);
    connect(ui->fontComboBox, qOverload<int>(&QFontComboBox::currentIndexChanged),
        this, &ShTextDialog::updateText);

    connect(ui->rb_bc, &QRadioButton::toggled, this, &ShTextDialog::updateText);
    connect(ui->rb_bl, &QRadioButton::toggled, this, &ShTextDialog::updateText);
    connect(ui->rb_br, &QRadioButton::toggled, this, &ShTextDialog::updateText);
    connect(ui->rb_c, &QRadioButton::toggled, this, &ShTextDialog::updateText);
    connect(ui->rb_lc, &QRadioButton::toggled, this, &ShTextDialog::updateText);
    connect(ui->rb_rc, &QRadioButton::toggled, this, &ShTextDialog::updateText);
    connect(ui->rb_tc, &QRadioButton::toggled, this, &ShTextDialog::updateText);
    connect(ui->rb_tl, &QRadioButton::toggled, this, &ShTextDialog::updateText);
    connect(ui->rb_tr, &QRadioButton::toggled, this, &ShTextDialog::updateText);
}

ShTextDialog::~ShTextDialog()
{
    delete ui;
}

void ShTextDialog::updateText()
{
    ui->textEdit->setFont(ui->fontComboBox->currentFont());
    ui->textEdit->update();
    ui->textEdit->repaint();

    pText->text = ui->textEdit->toPlainText();
    pText->font = ui->fontComboBox->currentFont().toString();
    pText->angle = ui->dsbxAngle->value();
    pText->height = ui->dsbxHeight->value();

    if (ui->rb_bc->isChecked()) {
        pText->centerAlign = 1;
    } else if (ui->rb_bl->isChecked()) {
        pText->centerAlign = 2;
    } else if (ui->rb_br->isChecked()) {
        pText->centerAlign = 3;
    } else if (ui->rb_c->isChecked()) {
        pText->centerAlign = 4;
    } else if (ui->rb_lc->isChecked()) {
        pText->centerAlign = 5;
    } else if (ui->rb_rc->isChecked()) {
        pText->centerAlign = 6;
    } else if (ui->rb_tc->isChecked()) {
        pText->centerAlign = 7;
    } else if (ui->rb_tl->isChecked()) {
        pText->centerAlign = 8;
    } else if (ui->rb_tr->isChecked()) {
        pText->centerAlign = 9;
    }

    qDebug() << pText->font;
    pText->redraw();
}
