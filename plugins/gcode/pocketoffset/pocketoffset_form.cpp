/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "pocketoffset_form.h"
#include "pocketoffset.h"
#include "ui_pocketoffsetform.h"

#include "graphicsview.h"
#include "settings.h"
#include <QMessageBox>
#include <span>

namespace PocketOffset {

enum {
    Offset,
    Raster,
};

Form::Form(GCode::Plugin* plugin)
    : GCode::Form{plugin, new Creator}
    , ui(new ::Ui::PocketOffsetForm)
    , names{tr("Pocket On"), tr("Pocket Outside"), tr("Pocket Inside")} {
    ui->setupUi(content);
    ui->toolHolder1->label()->setText(u"Tool 1:"_s);
    ui->toolHolder2->label()->setText(u"Tool 2:"_s);
    ui->toolHolder3->label()->setText(u"Tool 3:"_s);
    ui->toolHolder4->label()->setText(u"Tool 4:"_s);

    setWindowTitle(tr("Pocket Offset Toolpath"));

    MySettings settings;
    settings.beginGroup(u"PocketOffset"_s);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOutside);
    settings.getValue(ui->sbxSteps);
    settings.getValue(ui->sbxToolQty);
    settings.endGroup();

    rb_clicked();
    // clang-format off
    connect(ui->rbClimb,        &QRadioButton::clicked,        this, &Form::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked,        this, &Form::rb_clicked);
    connect(ui->rbInside,       &QRadioButton::clicked,        this, &Form::rb_clicked);
    connect(ui->rbOutside,      &QRadioButton::clicked,        this, &Form::rb_clicked);
    connect(ui->sbxToolQty,     &QSpinBox::valueChanged,       this, &Form::rb_clicked);

    connect(ui->toolHolder1,    &ToolSelectorForm::updateName, this, &Form::updateName);
    connect(ui->toolHolder2,    &ToolSelectorForm::updateName, this, &Form::updateName);
    connect(ui->toolHolder3,    &ToolSelectorForm::updateName, this, &Form::updateName);
    connect(ui->toolHolder4,    &ToolSelectorForm::updateName, this, &Form::updateName);

    connect(leName,             &QLineEdit::textChanged,       this, &Form::onNameTextChanged);
    connect(ui->sbxSteps,       &QSpinBox::valueChanged,       this, &Form::onSbxStepsValueChanged);
    // clang-format on
    if(ui->sbxSteps->value() == 0)
        ui->sbxSteps->setSuffix(tr(" - Infinity"));
    qDebug(__FUNCTION__);
}

Form::~Form() {
    MySettings settings;
    settings.beginGroup(u"PocketOffset"_s);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbOutside);
    settings.setValue(ui->sbxSteps);
    settings.setValue(ui->sbxToolQty);
    settings.endGroup();
    delete ui;
}

void Form::computePaths() {
    const Tool tool[]{
        ui->toolHolder1->tool(),
        ui->toolHolder2->tool(),
        ui->toolHolder3->tool(),
        ui->toolHolder4->tool(),
    };

    // Проверяются только задействованные держатели. Прежде проверялись все
    // четыре, и расчёт с одним инструментом не запускался вовсе: пустые
    // держатели 2..4 невалидны, а их виджеты при этом скрыты (см. rb_clicked).
    const auto qty = std::min<size_t>(std::max(ui->sbxToolQty->value(), 1), std::size(tool));
    for(const Tool& t: std::span{tool}.first(qty)) {
        if(!t.isValid()) {
            t.errorMessageBox(this);
            return;
        }
    }

    if(!getNewGcpWithGi()) return;

    gcp.tools.append_range(std::span{tool}.first(qty));

    { // sort and unique
        auto cmp = [this](const Tool& t1, const Tool& t2) -> bool {
            return t1.getDiameter(dsbxDepth->value()) > t2.getDiameter(dsbxDepth->value());
        };
        auto cmp2 = [this](const Tool& t1, const Tool& t2) -> bool {
            return qFuzzyCompare(t1.getDiameter(dsbxDepth->value()), t2.getDiameter(dsbxDepth->value()));
        };
        std::sort(gcp.tools.begin(), gcp.tools.end(), cmp);
        auto last = std::unique(gcp.tools.begin(), gcp.tools.end(), cmp2);
        gcp.tools.erase(last, gcp.tools.end());
    }

    gcp.setConvent(ui->rbConventional->isChecked());
    gcp.setSide(side);
    gcp.params[GCode::Params::Depth] = dsbxDepth->value();
    if(ui->sbxSteps->isVisible())
        gcp.params[Creator::OffsetSteps] = ui->sbxSteps->value();

    fileCount = static_cast<int>(gcp.tools.size());
    emit createToolpath(&gcp);
}

void Form::onSbxStepsValueChanged(int arg1) {
    ui->sbxSteps->setSuffix(!arg1 ? tr(" - Infinity") : QString{});
}

void Form::updateName() {
    leName->setText(names[side]);
}

void Form::updatePixmap() {
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[direction]).pixmap(QSize(150, 150)));
}

void Form::rb_clicked() {
    const auto tool{ui->toolHolder1->tool()};

    if(ui->rbOutside->isChecked())
        side = GCode::Outer;
    else if(ui->rbInside->isChecked())
        side = GCode::Inner;

    // if (tool.type() == Tool::Laser)
    // ui->chbxUseTwoTools->setChecked(false);

    if(ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if(ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    {
        int fl = ui->sbxToolQty->value();

        // ui->labelSteps->setVisible(fl < 2);
        // ui->sbxSteps->setVisible(fl < 2);

        ui->toolHolder2->setVisible(--fl > 0);
        ui->toolHolder3->setVisible(--fl > 0);
        ui->toolHolder4->setVisible(--fl > 0);
    }

    updateName();
    updateButtonIconSize();

    updatePixmap();
}

void Form::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void Form::showEvent(QShowEvent* event) {
    updatePixmap();
    // Базовая, а не QWidget: в GCode::Form::showEvent сбрасывается режим правки.
    GCode::Form::showEvent(event);
}

void Form::editFile(GCode::File* file) {
    const GCode::Params& gcp = file->params();

    // Радиокнопки и число инструментов первыми: их rb_clicked() зовёт
    // updateName() и перебил бы имя. Базовая ставит имя после этого.
    if(gcp.params.contains(GCode::Params::Side))
        (gcp.side() == GCode::Inner ? ui->rbInside : ui->rbOutside)->setChecked(true);
    if(gcp.params.contains(GCode::Params::Convent))
        (gcp.convent() ? ui->rbConventional : ui->rbClimb)->setChecked(true);

    // Инструменты -- копии, снятые при расчёте, уже отсортированные по убыванию
    // диаметра и без дублей. В базе инструментов их могло уже не остаться,
    // поэтому ставим как есть, не разыскивая по id.
    ToolSelectorForm* holders[]{ui->toolHolder1, ui->toolHolder2, ui->toolHolder3, ui->toolHolder4};
    const auto qty = std::min(gcp.tools.size(), std::size(holders));
    for(size_t i{}; i < qty; ++i) holders[i]->setTool(gcp.tools[i]);
    if(qty) ui->sbxToolQty->setValue(static_cast<int>(qty));

    if(auto it = gcp.params.find(Creator::OffsetSteps); it != gcp.params.end())
        ui->sbxSteps->setValue(static_cast<int>(it->second.toInt()));

    GCode::Form::editFile(file);
}

void Form::onNameTextChanged(const QString& arg1) { fileName_ = arg1; }

} // namespace PocketOffset

#include "moc_pocketoffset_form.cpp"
