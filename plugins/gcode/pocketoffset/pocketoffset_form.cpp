// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "pocketoffset_form.h"
#include "pocketoffset.h"
#include "ui_pocketoffsetform.h"

#include "scene.h"
#include "settings.h"
#include <QMessageBox>

enum {
    Offset,
    Raster,
};

PocketOffsetForm::PocketOffsetForm(GCodePlugin* plugin, QWidget* parent)
    : FormsUtil(plugin, new GCode::PocketCreator, parent)
    , ui(new Ui::PocketOffsetForm)
    , names {tr("Pockert On"), tr("Pocket Outside"), tr("Pocket Inside")} {
    ui->setupUi(content);
    ui->toolHolder->label()->setText("Tool 1:");
    ui->toolHolder2->label()->setText("Tool 2:");
    ui->toolHolder3->label()->setText("Tool 3:");
    ui->toolHolder4->label()->setText("Tool 4:");

    label->setText(tr("Pocket Offset Toolpath"));
    /*parent->*/ setWindowTitle(label->text());

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({16, 16});

    MySettings settings;
    settings.beginGroup("PocketOffsetForm");
    settings.getValue(ui->sbxToolQty);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOutside);
    settings.getValue(ui->sbxSteps);
    settings.endGroup();

    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &PocketOffsetForm::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &PocketOffsetForm::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &PocketOffsetForm::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &PocketOffsetForm::rb_clicked);
    connect(ui->sbxToolQty, qOverload<int>(&QSpinBox::valueChanged), this, &PocketOffsetForm::rb_clicked);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &PocketOffsetForm::updateName);
    connect(ui->toolHolder2, &ToolSelectorForm::updateName, this, &PocketOffsetForm::updateName);
    connect(ui->toolHolder3, &ToolSelectorForm::updateName, this, &PocketOffsetForm::updateName);
    connect(ui->toolHolder4, &ToolSelectorForm::updateName, this, &PocketOffsetForm::updateName);

    connect(pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);
    connect(pbCreate, &QPushButton::clicked, this, &PocketOffsetForm::createFile);

    ui->sbxSteps->setSuffix(tr(" - Infinity"));
}

PocketOffsetForm::~PocketOffsetForm() {

    MySettings settings;
    settings.beginGroup("PocketOffsetForm");
    settings.setValue(ui->sbxToolQty);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbOutside);
    settings.setValue(ui->sbxSteps);
    settings.endGroup();
    delete ui;
}

void PocketOffsetForm::createFile() {
    const Tool tool[] {
        ui->toolHolder->tool(),
        ui->toolHolder2->tool(),
        ui->toolHolder3->tool(),
        ui->toolHolder4->tool()};

    for (const Tool& t : tool) {
        if (!t.isValid()) {
            t.errorMessageBox(this);
            return;
        }
    }

    //    if (ui->chbxUseTwoTools->isChecked() && !tool2.isValid()) {
    //        tool2.errorMessageBox(this);
    //        return;
    //    }

    Paths wPaths;
    Paths wRawPaths;
    FileInterface const* file = nullptr;
    bool skip {true};

    for (auto* item : App::scene()->selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        switch (static_cast<GiType>(item->type())) {
        case GiType::DataSolid:
        case GiType::DataPath:
            if (!file) {
                file = gi->file();
                boardSide = file->side();
            } else if (file != gi->file()) {
                if (skip) {
                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
                        return;
                }
            }
            if (static_cast<GiType>(item->type()) == GiType::DataSolid)
                wPaths.append(gi->paths());
            else
                wRawPaths.append(gi->paths());
            break;
        case GiType::ShCircle:
        case GiType::ShRectangle:
        case GiType::ShPolyLine:
        case GiType::ShCirArc:
        case GiType::ShText:
            wRawPaths.append(gi->paths());
            break;
        case GiType::Drill:
            wPaths.append(gi->paths());
            break;
        default:
            break;
        }
        addUsedGi(gi);
    }

    if (wRawPaths.empty() && wPaths.empty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::GCodeParams gcp_;
    for (const Tool& t : tool) {
        gcp_.tools.push_back(t);
        if (gcp_.tools.size() == static_cast<size_t>(ui->sbxToolQty->value()))
            break;
    }

    { // sort and unique
        auto cmp = [this](const Tool& t1, const Tool& t2) -> bool {
            return t1.getDiameter(dsbxDepth->value()) > t2.getDiameter(dsbxDepth->value());
        };
        auto cmp2 = [this](const Tool& t1, const Tool& t2) -> bool {
            return qFuzzyCompare(t1.getDiameter(dsbxDepth->value()), t2.getDiameter(dsbxDepth->value()));
        };
        std::sort(gcp_.tools.begin(), gcp_.tools.end(), cmp);
        auto last = std::unique(gcp_.tools.begin(), gcp_.tools.end(), cmp2);
        gcp_.tools.erase(last, gcp_.tools.end());
    }

    gcp_.setConvent(ui->rbConventional->isChecked());
    gcp_.setSide(side);
    gcp_.params[GCode::GCodeParams::Depth] = dsbxDepth->value();
    if (ui->sbxSteps->isVisible())
        gcp_.params[GCode::GCodeParams::Steps] = ui->sbxSteps->value();

    creator->setGcp(gcp_);
    creator->addPaths(wPaths);
    creator->addRawPaths(wRawPaths);
    fileCount = static_cast<int>(gcp_.tools.size());
    createToolpath();
}

void PocketOffsetForm::on_sbxSteps_valueChanged(int arg1) {
    ui->sbxSteps->setSuffix(!arg1 ? tr(" - Infinity") : "");
}

void PocketOffsetForm::updateName() {
    leName->setText(names[side]);
}

void PocketOffsetForm::updatePixmap() {
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[direction]).pixmap(QSize(150, 150)));
}

void PocketOffsetForm::rb_clicked() {
    const auto tool {ui->toolHolder->tool()};

    if (ui->rbOutside->isChecked())
        side = GCode::Outer;
    else if (ui->rbInside->isChecked())
        side = GCode::Inner;

    //    if (tool.type() == Tool::Laser)
    //        ui->chbxUseTwoTools->setChecked(false);

    if (ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if (ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    {
        int fl = ui->sbxToolQty->value();

        //        ui->labelSteps->setVisible(fl < 2);
        //        ui->sbxSteps->setVisible(fl < 2);

        ui->toolHolder2->setVisible(--fl > 0);
        ui->toolHolder3->setVisible(--fl > 0);
        ui->toolHolder4->setVisible(--fl > 0);
    }

    updateName();
    updatePixmap();
}

void PocketOffsetForm::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void PocketOffsetForm::showEvent(QShowEvent* event) {
    updatePixmap();
    QWidget::showEvent(event);
}

void PocketOffsetForm::on_leName_textChanged(const QString& arg1) { fileName_ = arg1; }

void PocketOffsetForm::editFile(GCode::File* /*file*/) {
}
