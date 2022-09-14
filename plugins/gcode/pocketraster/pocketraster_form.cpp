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

#include "pocketraster_form.h"
#include "pocketraster.h"
#include "ui_pocketrasterform.h"

#include "graphicsview.h"
#include "settings.h"
#include <QMessageBox>

PocketRasterForm::PocketRasterForm(GCodePlugin* plugin, QWidget* parent)
    : GcFormBase(plugin, new GCode::RasterCreator, parent)
    , ui(new Ui::PocketRasterForm)
    , names {tr("Raster On"), tr("Raster Outside"), tr("Raster Inside")} {
    ui->setupUi(content);

    setWindowTitle(tr("Pocket Raster Toolpath"));

    MySettings settings;
    settings.beginGroup("PocketRasterForm");
    settings.getValue(ui->cbxPass);
    settings.getValue(ui->dsbxAcc);
    settings.getValue(ui->dsbxAngle);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbFast);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbNormal);
    settings.getValue(ui->rbOutside);
    settings.endGroup();

    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &PocketRasterForm::updateName);

    connect(leName, &QLineEdit::textChanged, this, &PocketRasterForm::onNameTextChanged);

    //
}

PocketRasterForm::~PocketRasterForm() {

    MySettings settings;
    settings.beginGroup("PocketRasterForm");
    settings.setValue(ui->cbxPass);
    settings.setValue(ui->dsbxAcc);
    settings.setValue(ui->dsbxAngle);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbFast);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbNormal);
    settings.setValue(ui->rbOutside);
    settings.endGroup();
    delete ui;
}

void PocketRasterForm::createFile() {
    const auto tool {ui->toolHolder->tool()};

    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    FileInterface const* file = nullptr;
    bool skip {true};

    for (auto* item : App::graphicsView()->scene()->selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        switch (item->type()) {
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
            if (item->type() == GiType::DataSolid)
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
    gcp_.setConvent(ui->rbConventional->isChecked());
    gcp_.setSide(side);
    gcp_.tools.push_back(tool);

    gcp_.params[GCode::GCodeParams::UseAngle] = ui->dsbxAngle->value();
    gcp_.params[GCode::GCodeParams::Depth] = dsbxDepth->value();
    gcp_.params[GCode::GCodeParams::Pass] = ui->cbxPass->currentIndex();
    if (ui->rbFast->isChecked()) {
        gcp_.params[GCode::GCodeParams::Fast] = true;
        gcp_.params[GCode::GCodeParams::AccDistance] = (tool.feedRate_mm_s() * tool.feedRate_mm_s()) / (2 * ui->dsbxAcc->value());
    }

    creator->setGcp(gcp_);
    creator->addPaths(wPaths);
    creator->addRawPaths(wRawPaths);
    fileCount = 1;
    createToolpath();
}

void PocketRasterForm::updateName() {
    const auto& tool {ui->toolHolder->tool()};
    if (tool.type() != Tool::Laser)
        ui->rbNormal->setChecked(true);
    ui->rbFast->setEnabled(tool.type() == Tool::Laser);

    leName->setText(names[side]);
}

void PocketRasterForm::updatePixmap() {
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[direction]).pixmap(QSize(150, 150)));
}

void PocketRasterForm::rb_clicked() {

    if (ui->rbOutside->isChecked())
        side = GCode::Outer;
    else if (ui->rbInside->isChecked())
        side = GCode::Inner;

    if (ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if (ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    updateName();
    updateButtonIconSize();

    updatePixmap();
}

void PocketRasterForm::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void PocketRasterForm::showEvent(QShowEvent* event) {
    updatePixmap();
    QWidget::showEvent(event);
}

void PocketRasterForm::onNameTextChanged(const QString& arg1) { fileName_ = arg1; }

void PocketRasterForm::editFile(GCode::File* /*file*/) {
}

#include "moc_pocketraster_form.cpp"
