/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "editor.h"
#include "doublespinbox.h"
#include "shape.h"

#include <QtWidgets>
#include <set>

Q_DECLARE_METATYPE(std::set<double>)

#define TR QCoreApplication::translate

namespace ShTxt {

auto filter = std::views::filter([](Shape* shape) {
    return shape->isSelected()
        // && shape->isEditable()
        ;
});

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
        auto gridLayout = new QGridLayout{groupBox};
        gridLayout->setContentsMargins(6, 6, 6, 6);
        gridLayout->addWidget(rb_tl = new QRadioButton{groupBox}, 0, 0 + 1);
        gridLayout->addWidget(rb_tc = new QRadioButton{groupBox}, 0, 1 + 1);
        gridLayout->addWidget(rb_tr = new QRadioButton{groupBox}, 0, 2 + 1);
        gridLayout->addWidget(rb_lc = new QRadioButton{groupBox}, 1, 0 + 1);
        gridLayout->addWidget(rb_cc = new QRadioButton{groupBox}, 1, 1 + 1);
        gridLayout->addWidget(rb_rc = new QRadioButton{groupBox}, 1, 2 + 1);
        gridLayout->addWidget(rb_bl = new QRadioButton{groupBox}, 2, 0 + 1);
        gridLayout->addWidget(rb_bc = new QRadioButton{groupBox}, 2, 1 + 1);
        gridLayout->addWidget(rb_br = new QRadioButton{groupBox}, 2, 2 + 1);
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
    formLayout->addRow(new QLabel{TR("TextEditor", "Pos. X:", nullptr), this}, dsbxX = new DoubleSpinBox{this});
    formLayout->addRow(new QLabel{TR("TextEditor", "Pos. Y:", nullptr), this}, dsbxY = new DoubleSpinBox{this});
    formLayout->addRow(new QLabel{TR("TextEditor", "Side:", nullptr), this}, cbxSide = new QComboBox{this});

    cbxSide->addItems(TR("TextEditor", "Top|Bottom").split('|'));

    dsbxAngle->setDecimals(0);
    dsbxAngle->setMaximum(360.0);
    dsbxAngle->setSuffix(TR("TextEditor", " \302\260", nullptr));

    dsbxHeight->setMaximum(100.0);
    dsbxHeight->setSuffix(TR("TextEditor", " mm", nullptr));

    dsbxXY->setDecimals(3);
    dsbxXY->setRange(0.001, 1000.0);
    dsbxXY->setSuffix(TR("TextEditor", " %", nullptr));
    dsbxXY->setValue(100.0);

    dsbxX->setDecimals(3);
    dsbxX->setRange(-1000.0, +1000.0);
    dsbxX->setSuffix(TR("TextEditor", " mm", nullptr));
    dsbxX->setValue(100.0);

    dsbxY->setDecimals(3);
    dsbxY->setRange(-1000.0, +1000.0);
    dsbxY->setSuffix(TR("TextEditor", " mm", nullptr));
    dsbxY->setValue(100.0);

    // clang-format off
    connect(plainTextEdit, &QPlainTextEdit::textChanged, this, &Editor::updateText);
    connect(dsbxAngle, &QDoubleSpinBox::valueChanged,    this, &Editor::updateAngle);
    connect(dsbxHeight, &QDoubleSpinBox::valueChanged,   this, &Editor::updateHeight);
    connect(dsbxXY, &QDoubleSpinBox::valueChanged,       this, &Editor::updateXY);
    connect(dsbxX, &QDoubleSpinBox::valueChanged,        this, &Editor::updateX);
    connect(dsbxY, &QDoubleSpinBox::valueChanged,        this, &Editor::updateY);
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

    // Apply
    auto pushButton = new QPushButton{TR("TextEditor", "Apply"), this};
    pushButton->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    formLayout->addRow(pushButton);
    connect(pushButton, &QPushButton::clicked, plugin, &Shapes::Plugin::finalizeShape);
    connect(pushButton, &QPushButton::clicked, this, [this] {
        for(auto* shape: shapes | filter) {
            shape->iDataCopy = shape->iData;
            shape->ok();
        }
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
    if(resetFl) return;
    QString text = plainTextEdit->toPlainText();
    for(auto* shape: shapes | filter) {
        shape->iData.text = text;
        shape->redraw();
    }
}

void Editor::updateFont() {
    if(resetFl) return;
    auto font{cbxFont->currentFont()};
    font.setBold(chbxBold->isChecked());
    font.setItalic(chbxItalic->isChecked());
    plainTextEdit->setFont(font);
    for(auto* shape: shapes | filter) {
        shape->iData.font = font;
        shape->redraw();
    }
}

void Editor::updateAngle() {
    if(resetFl) return;
    for(auto* shape: shapes | filter) {
        shape->iData.angle = dsbxAngle->value();
        shape->redraw();
    }
}

void Editor::updateHeight() {
    if(resetFl) return;
    for(auto* shape: shapes | filter) {
        shape->iData.height = dsbxHeight->value();
        shape->redraw();
    }
}

void Editor::updateXY() {
    if(resetFl) return;
    for(auto* shape: shapes | filter) {
        shape->iData.xy = dsbxXY->value();
        shape->redraw();
    }
}

void Editor::updateX() {
    if(resetFl) return;
    for(auto* shape: shapes | filter) {
        shape->handles.front().setX(dsbxX->value());
        shape->redraw();
    }
}

void Editor::updateY() {
    if(resetFl) return;
    for(auto* shape: shapes | filter) {
        shape->handles.front().setY(dsbxY->value());
        shape->redraw();
    }
}

void Editor::updateCenterAlign() {
    if(resetFl) return;

    int handleAlign{};

    if(rb_bc->isChecked()) handleAlign = Shape::BotCenter;
    else if(rb_bl->isChecked()) handleAlign = Shape::BotLeft;
    else if(rb_br->isChecked()) handleAlign = Shape::BotRight;
    else if(rb_cc->isChecked()) handleAlign = Shape::Center;
    else if(rb_lc->isChecked()) handleAlign = Shape::CenterLeft;
    else if(rb_rc->isChecked()) handleAlign = Shape::CenterRight;
    else if(rb_tc->isChecked()) handleAlign = Shape::TopCenter;
    else if(rb_tl->isChecked()) handleAlign = Shape::TopLeft;
    else if(rb_tr->isChecked()) handleAlign = Shape::TopRight;

    for(auto* shape: shapes | filter) {
        shape->iData.handleAlign = handleAlign;
        shape->redraw();
    }
}

void Editor::updateSide() {
    if(resetFl) return;

    for(auto* shape: shapes | filter) {
        shape->iData.side = static_cast<Side>(cbxSide->currentIndex());
        shape->redraw();
    }
}

Editor::Editor(Shapes::Plugin* plugin)
    : plugin{plugin} {
    setWindowTitle(plugin->name());

    setupUi();
}

void Editor::add(Shapes::AbstractShape* shape) {
    shapes.emplace_back(static_cast<Shape*>(shape));
    reset();
}

void Editor::remove(Shapes::AbstractShape* shape) {
    std::erase(shapes, static_cast<Shape*>(shape));
    // view->reset();
}

void Editor::reset() {
    if(!isVisible() || shapes.empty()) return;
    resetFl = true;

    for(auto* shape: shapes) shape->save();

    cbxFont->setFontFilters(
        QFontComboBox::ScalableFonts
        //        | QFontComboBox::NonScalableFonts
        | QFontComboBox::MonospacedFonts
        | QFontComboBox::ProportionalFonts);

    auto filtered = shapes | filter;
    auto& first = filtered.empty()
        ? shapes.front()->iData
        : filtered.front()->iData;

    if(qFuzzyIsNull(first.height)) first.height = 10.0;
    if(qFuzzyIsNull(first.xy)) first.xy = 100.0;

    {
        QFont font = first.font.family();
        cbxFont->setCurrentFont(font);
        plainTextEdit->setFont(font);
        chbxBold->setChecked(font.bold());
        chbxItalic->setChecked(font.italic());
    }

    cbxSide->setCurrentIndex(static_cast<int>(first.side));
    plainTextEdit->setPlainText(first.text);
    dsbxAngle->setValue(first.angle);
    dsbxHeight->setValue(first.height);
    dsbxXY->setValue(first.xy);

    switch(first.handleAlign) {
        // clang-format off
    case Shape::BotCenter:   rb_bc->setChecked(true); break;
    case Shape::BotLeft:     rb_bl->setChecked(true); break;
    case Shape::BotRight:    rb_br->setChecked(true); break;
    case Shape::Center:      rb_cc->setChecked(true); break;
    case Shape::CenterLeft:  rb_lc->setChecked(true); break;
    case Shape::CenterRight: rb_rc->setChecked(true); break;
    case Shape::TopCenter:   rb_tc->setChecked(true); break;
    case Shape::TopLeft:     rb_tl->setChecked(true); break;
    case Shape::TopRight:    rb_tr->setChecked(true); break;
        // clang-format on
    }
    resetFl = false;
}

void Editor::hideEvent(QHideEvent* event) {
    for(auto* shape: shapes | filter) shape->restore();
    QWidget::hideEvent(event);
}

void Editor::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    reset();
}

} // namespace ShTxt

#include "moc_editor.cpp"
