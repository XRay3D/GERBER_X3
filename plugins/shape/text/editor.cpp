// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "editor.h"
#include "doublespinbox.h"
#include "shape.h"
#include "shhandler.h"

#include <QtWidgets>
#include <array>
#include <set>

Q_DECLARE_METATYPE(std::set<double>)

#define TR QCoreApplication::translate

namespace ShTxt {

//////////////////////////////////////////
/// \brief Editor::Editor
void Editor::setupUi() {

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    setSizePolicy(sizePolicy);
    auto formLayout = new QFormLayout{this};
    formLayout->setContentsMargins(6, 6, 6, 6);
    formLayout->setLabelAlignment(Qt::AlignRight);

    formLayout->addRow(plainTextEdit = new QPlainTextEdit{this});

    {
        auto groupBox = new QWidget{this};
        auto gridLayout = new QGridLayout(groupBox);
        gridLayout->setContentsMargins(6, 6, 6, 6);
        gridLayout->addWidget(rb_tl = new QRadioButton(groupBox), 0, 0 + 1);
        gridLayout->addWidget(rb_tc = new QRadioButton(groupBox), 0, 1 + 1);
        gridLayout->addWidget(rb_tr = new QRadioButton(groupBox), 0, 2 + 1);
        gridLayout->addWidget(rb_lc = new QRadioButton(groupBox), 1, 0 + 1);
        gridLayout->addWidget(rb_cc = new QRadioButton(groupBox), 1, 1 + 1);
        gridLayout->addWidget(rb_rc = new QRadioButton(groupBox), 1, 2 + 1);
        gridLayout->addWidget(rb_bl = new QRadioButton(groupBox), 2, 0 + 1);
        gridLayout->addWidget(rb_bc = new QRadioButton(groupBox), 2, 1 + 1);
        gridLayout->addWidget(rb_br = new QRadioButton(groupBox), 2, 2 + 1);
        gridLayout->setColumnStretch(0, 1);
        gridLayout->setColumnStretch(4, 1);
        formLayout->addRow(new QLabel{TR("TextEditor", "Align:", nullptr), this}, groupBox);
        rb_bl->setChecked(true);
    }

    formLayout->addRow(new QLabel{TR("TextEditor", "Font:", nullptr), this}, cbxFont = new QFontComboBox{this});
    formLayout->addRow(new QLabel{TR("TextEditor", "Bold:", nullptr), this}, chbxBold = new QCheckBox{this});
    formLayout->addRow(new QLabel{TR("TextEditor", "Italic:", nullptr), this}, chbxItalic = new QCheckBox{this});
    formLayout->addRow(new QLabel{TR("TextEditor", "Angle:", nullptr), this}, dsbxAngle = new DoubleSpinBox{this});
    formLayout->addRow(new QLabel{TR("TextEditor", "Height:", nullptr), this}, dsbxHeight = new DoubleSpinBox{this});
    formLayout->addRow(new QLabel{TR("TextEditor", "X/Y:", nullptr), this}, dsbxXY = new DoubleSpinBox{this});
    formLayout->addRow(new QLabel{TR("TextEditor", "Side:", nullptr), this}, cbxSide = new QComboBox{this});

    cbxSide->addItems(TR("TextEditor", "Top|Bottom").split('|'));

    dsbxAngle->setDecimals(0);
    dsbxAngle->setMaximum(360.0);
    dsbxAngle->setSuffix(TR("TextEditor", " \302\260", nullptr));

    dsbxHeight->setMaximum(100.0);
    dsbxHeight->setSuffix(TR("TextEditor", " mm", nullptr));

    dsbxXY->setDecimals(3);
    dsbxXY->setMaximum(1000.0);
    dsbxXY->setMinimum(0.001);
    dsbxXY->setSuffix(TR("TextEditor", " %", nullptr));
    dsbxXY->setValue(100.0);

    // Apply
    auto pushButton = new QPushButton{TR("TextEditor", "Apply"), this};
    pushButton->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    formLayout->addRow(pushButton);
    connect(pushButton, &QPushButton::clicked, plugin, &Shapes::Plugin::finalizeShape);
    connect(pushButton, &QPushButton::clicked, this, [this] {
        auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
        for(auto text: sh)
            text->iDataCopy = text->iData;
    });

    // Add New
    pushButton = new QPushButton{TR("TextEditor", "Add New"), this};
    pushButton->setIcon(QIcon::fromTheme("list-add"));
    formLayout->addRow(pushButton);
    connect(pushButton, &QPushButton::clicked, this, [this] {
        plugin->finalizeShape();
        App::project().addShape(plugin->createShape());
    });

    // Close
    pushButton = new QPushButton{"Close", this};
    pushButton->setObjectName("pbClose");
    pushButton->setIcon(QIcon::fromTheme("window-close"));
    formLayout->addRow(pushButton);
}

void Editor::updateText() {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    QString text_(plainTextEdit->toPlainText());
    for(auto text: sh) {
        text->iData.text = text_;
        text->redraw();
    }
}

void Editor::updateFont() {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });

    auto font{cbxFont->currentFont()};
    font.setBold(chbxBold->isChecked());
    font.setItalic(chbxItalic->isChecked());
    plainTextEdit->setFont(font);
    auto strFont(font.toString());
    for(auto text: sh) {
        text->iData.font = strFont;
        text->redraw();
    }
}

void Editor::updateAngle() {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });

    for(auto text: sh) {
        text->iData.angle = dsbxAngle->value();
        text->redraw();
    }
}

void Editor::updateHeight() {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });

    for(auto text: sh) {
        text->iData.height = dsbxHeight->value();
        text->redraw();
    }
}

void Editor::updateXY() {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });

    for(auto text: sh) {
        text->iData.xy = dsbxXY->value();
        text->redraw();
    }
}

void Editor::updateCenterAlign() {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });

    int handleAlign;
    if(rb_bc->isChecked())
        handleAlign = Shape::BotCenter;
    else if(rb_bl->isChecked())
        handleAlign = Shape::BotLeft;
    else if(rb_br->isChecked())
        handleAlign = Shape::BotRight;
    else if(rb_cc->isChecked())
        handleAlign = Shape::Center;
    else if(rb_lc->isChecked())
        handleAlign = Shape::CenterLeft;
    else if(rb_rc->isChecked())
        handleAlign = Shape::CenterRight;
    else if(rb_tc->isChecked())
        handleAlign = Shape::TopCenter;
    else if(rb_tl->isChecked())
        handleAlign = Shape::TopLeft;
    else if(rb_tr->isChecked())
        handleAlign = Shape::TopRight;
    for(auto text: sh) {
        text->iData.handleAlign = handleAlign;
        text->redraw();
    }
}

void Editor::updateSide() {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });

    for(auto text: sh) {
        text->iData.side = static_cast<Side>(cbxSide->currentIndex());
        text->redraw();
    }
}

Editor::Editor(Plugin* plugin)
    : plugin{plugin} {
    setWindowTitle(plugin->name());

    setupUi();
}

void Editor::addShape(Shape* shape) {
    shapes.emplace_back(shape);
    shape->editor = this;
    reset();
}

void Editor::reset() {
    if(!isVisible()) return;

    // clang-format off
    disconnect(plainTextEdit, &QPlainTextEdit::textChanged, this, &Editor::updateText);
    disconnect(dsbxAngle, &QDoubleSpinBox::valueChanged,    this, &Editor::updateAngle);
    disconnect(dsbxHeight, &QDoubleSpinBox::valueChanged,   this, &Editor::updateHeight);
    disconnect(dsbxXY, &QDoubleSpinBox::valueChanged,       this, &Editor::updateXY);
    disconnect(cbxFont, &QFontComboBox::currentFontChanged, this, &Editor::updateFont);
    disconnect(cbxSide, &QComboBox::currentIndexChanged,    this, &Editor::updateSide);

    disconnect(rb_bc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    disconnect(rb_bl, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    disconnect(rb_br, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    disconnect(rb_cc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    disconnect(rb_lc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    disconnect(rb_rc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    disconnect(rb_tc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    disconnect(rb_tl, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    disconnect(rb_tr, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);

    disconnect(chbxBold, &QCheckBox::toggled,               this, &Editor::updateFont);
    disconnect(chbxItalic, &QCheckBox::toggled,             this, &Editor::updateFont);
    // clang-format on

    for(auto text: shapes)
        text->save();

    //    plainTextEdit->setStyleSheet("QPlainTextEdit { font-size: 32pt }");

    cbxFont->setFontFilters(
        QFontComboBox::ScalableFonts
        //        | QFontComboBox::NonScalableFonts
        | QFontComboBox::MonospacedFonts
        | QFontComboBox::ProportionalFonts);

    {
        QFont font;
        font.fromString(shapes.front()->iData.font);
        cbxFont->setCurrentFont(font);
        plainTextEdit->setFont(font);
        chbxBold->setChecked(font.bold());
        chbxItalic->setChecked(font.italic());
    }

    cbxSide->setCurrentIndex(static_cast<int>(shapes.front()->iData.side));
    plainTextEdit->setPlainText(shapes.front()->iData.text);
    dsbxAngle->setValue(shapes.front()->iData.angle);
    dsbxHeight->setValue(shapes.front()->iData.height);
    dsbxXY->setValue(shapes.front()->iData.xy);

    // clang-format off
    switch(shapes.front()->iData.handleAlign) {
    case Shape::BotCenter:   rb_bc->setChecked(true); break;
    case Shape::BotLeft:     rb_bl->setChecked(true); break;
    case Shape::BotRight:    rb_br->setChecked(true); break;
    case Shape::Center:      rb_cc->setChecked(true); break;
    case Shape::CenterLeft:  rb_lc->setChecked(true); break;
    case Shape::CenterRight: rb_rc->setChecked(true); break;
    case Shape::TopCenter:   rb_tc->setChecked(true); break;
    case Shape::TopLeft:     rb_tl->setChecked(true); break;
    case Shape::TopRight:    rb_tr->setChecked(true); break;
    }

    // clang-format off
    connect(plainTextEdit, &QPlainTextEdit::textChanged, this, &Editor::updateText);
    connect(dsbxAngle, &QDoubleSpinBox::valueChanged,    this, &Editor::updateAngle);
    connect(dsbxHeight, &QDoubleSpinBox::valueChanged,   this, &Editor::updateHeight);
    connect(dsbxXY, &QDoubleSpinBox::valueChanged,       this, &Editor::updateXY);
    connect(cbxFont, &QFontComboBox::currentFontChanged, this, &Editor::updateFont);
    connect(cbxSide, &QComboBox::currentIndexChanged,    this, &Editor::updateSide);

    connect(rb_bc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    connect(rb_bl, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    connect(rb_br, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    connect(rb_cc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    connect(rb_lc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    connect(rb_rc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    connect(rb_tc, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    connect(rb_tl, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);
    connect(rb_tr, &QRadioButton::toggled,               this, &Editor::updateCenterAlign);

    connect(chbxBold, &QCheckBox::toggled,               this, &Editor::updateFont);
    connect(chbxItalic, &QCheckBox::toggled,             this, &Editor::updateFont);
    // clang-format on
}

void Editor::hideEvent(QHideEvent* event) {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    for(auto text: sh)
        text->restore();
    QWidget::hideEvent(event);
}

} // namespace ShTxt

#include "moc_editor.cpp"
