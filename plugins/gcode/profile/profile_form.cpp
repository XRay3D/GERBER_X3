// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "profile_form.h"
#include "profile.h"
#include "ui_profileform.h"

#include "gi_bridge.h"
#include "graphicsview.h"
#include "settings.h"
#include <QMessageBox>

ProfileForm::ProfileForm(GCodePlugin* plugin, QWidget* parent)
    : GcFormBase(plugin, new GCode::ProfileCtr, parent)
    , ui(new Ui::ProfileForm) {
    ui->setupUi(content);
    setWindowTitle(tr("Profile Toolpath"));

    ui->pbAddBridge->setIcon(QIcon::fromTheme("edit-cut"));

    MySettings settings;
    settings.beginGroup("ProfileForm");
    settings.getValue(ui->dsbxBridgeLenght, 1.0);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOn);
    settings.getValue(ui->rbOutside);
    settings.getValue(ui->cbxTrimming);
    settings.getValue(varName(trimming_), 0);
    settings.endGroup();

    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbOn, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);

    connect(ui->dsbxBridgeLenght, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ProfileForm::updateBridge);
    connect(dsbxDepth, &DepthForm::valueChanged, this, &ProfileForm::updateBridge);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &ProfileForm::updateName);

    connect(leName, &QLineEdit::textChanged, this, &ProfileForm::onNameTextChanged);

    //
    connect(ui->cbxTrimming, &QCheckBox::toggled, [this](bool checked) {
        if (side == GCode::On)
            checked ? trimming_ |= Trimming::Line : trimming_ &= ~Trimming::Line;
        else
            checked ? trimming_ |= Trimming::Corner : trimming_ &= ~Trimming::Corner;
    });

    connect(ui->pbAddBridge, &QPushButton::clicked, this, &ProfileForm::onAddBridgeClicked);
}

ProfileForm::~ProfileForm() {
    MySettings settings;
    settings.beginGroup("ProfileForm");
    settings.setValue(ui->dsbxBridgeLenght);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbOn);
    settings.setValue(ui->rbOutside);
    settings.setValue(ui->cbxTrimming);
    settings.setValue(varName(trimming_));
    settings.endGroup();

    for (QGraphicsItem* giItem : App::graphicsView()->scene()->items()) {
        if (giItem->type() == GiType::Bridge)
            delete giItem;
    }
    delete ui;
}

void ProfileForm::createFile() {
    usedItems_.clear();
    const auto tool {ui->toolHolder->tool()};
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    FileInterface const* file = nullptr;
    bool skip {true};

    for (auto* sItem : App::graphicsView()->scene()->selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(sItem);
        switch (sItem->type()) {
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
            if (sItem->type() == GiType::DataSolid)
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
    gcp_.params[GCode::GCodeParams::Depth] = dsbxDepth->value();

    if (side == GCode::On)
        gcp_.params[GCode::ProfileCtr::TrimmingOpenPaths] = ui->cbxTrimming->isChecked();
    else
        gcp_.params[GCode::ProfileCtr::TrimmingCorners] = ui->cbxTrimming->isChecked();

    gcp_.params[GCode::GCodeParams::GrItems].setValue(usedItems_);

    QPolygonF brv;
    for (QGraphicsItem* item : App::graphicsView()->scene()->items()) {
        if (item->type() == GiType::Bridge)
            brv.push_back(item->pos());
    }
    if (!brv.isEmpty()) {
        // gcp_.params[GCode::GCodeParams::Bridges].fromValue(brv);
        gcp_.params[GCode::ProfileCtr::BridgeLen] = ui->dsbxBridgeLenght->value();
    }

    gcCreator->setGcp(gcp_);
    gcCreator->addPaths(wPaths);
    gcCreator->addRawPaths(wRawPaths);
    fileCount = 1;
    emit createToolpath();
}

void ProfileForm::updateName() {
    leName->setText(names[side]);
    updateBridge();
}

void ProfileForm::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void ProfileForm::showEvent(QShowEvent* event) {
    updatePixmap();
    QWidget::showEvent(event);
}

void ProfileForm::onAddBridgeClicked() {
    if (brItem) {
        if (!brItem->ok())
            delete brItem;
    }
    brItem = new GiBridge(lenght_, size_, side, brItem);
    App::graphicsView()->scene()->addItem(brItem);
    brItem->setVisible(true);
    brItem->setOpacity(1.0);
}

void ProfileForm::updateBridge() {
    lenght_ = ui->dsbxBridgeLenght->value();
    size_ = ui->toolHolder->tool().getDiameter(dsbxDepth->value());
    for (QGraphicsItem* item : App::graphicsView()->scene()->items()) {
        if (item->type() == GiType::Bridge)
            static_cast<GiBridge*>(item)->update();
    }
}

void ProfileForm::updatePixmap() {
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[side + direction * 3]).pixmap(QSize(size, size)));
}

void ProfileForm::rb_clicked() {
    if (ui->rbOn->isChecked()) {
        side = GCode::On;
        ui->cbxTrimming->setText(tr("Trimming"));
        ui->cbxTrimming->setChecked(trimming_ & Trimming::Line);
    } else if (ui->rbOutside->isChecked()) {
        side = GCode::Outer;
        ui->cbxTrimming->setText(tr("Corner Trimming"));
        ui->cbxTrimming->setChecked(trimming_ & Trimming::Corner);
    } else if (ui->rbInside->isChecked()) {
        side = GCode::Inner;
        ui->cbxTrimming->setText(tr("Corner Trimming"));
        ui->cbxTrimming->setChecked(trimming_ & Trimming::Corner);
    }

    if (ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if (ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    updateName();
    updateButtonIconSize();

    updatePixmap();
}

void ProfileForm::onNameTextChanged(const QString& arg1) { fileName_ = arg1; }

void ProfileForm::editFile(GCode::File* file) {

    //    GCode::GCodeParams gcp_ {file->gcp()};

    //    fileId = gcp_.fileId;
    //    editMode_ = true;

    //    { // GUI
    //        side = gcp_.side();
    //        direction = static_cast<GCode::Direction>(gcp_.convent());
    //        ui->toolHolder->setTool(gcp_.tools.front());
    //        dsbxDepth->setValue(gcp_.params[GCode::GCodeParams::Depth].toDouble());

    //        switch (side) {
    //        case GCode::On:
    //            ui->rbOn->setChecked(true);
    //            break;
    //        case GCode::Outer:
    //            ui->rbOutside->setChecked(true);
    //            break;
    //        case GCode::Inner:
    //            ui->rbInside->setChecked(true);
    //            break;
    //        }

    //        switch (direction) {
    //        case GCode::Climb:
    //            ui->rbClimb->setChecked(true);
    //            break;
    //        case GCode::Conventional:
    //            ui->rbConventional->setChecked(true);
    //            break;
    //        }
    //    }

    //    { // GrItems
    //        usedItems_.clear();
    //        auto items {gcp_.params[GCode::GCodeParams::GrItems].value<UsedItems>()};

    //        auto i = items.cbegin();
    //        while (i != items.cend()) {

    //            //            auto [_fileId, _] = i.key();
    //            //            Q_UNUSED(_)
    //            //            App::project()->file(_fileId)->itemGroup()->setSelected(i.value());
    //            //            ++i;
    //        }
    //    }

    //    { // Bridges
    //        if (gcp_.params.contains(GCode::GCodeParams::Bridges)) {
    //            ui->dsbxBridgeLenght->setValue(gcp_.params[GCode::GCodeParams::BridgeLen].toDouble());
    //            //            for (auto& pos : gcp_.params[GCode::GCodeParams::Bridges].value<QPolygonF>()) {
    //            //                brItem = new BridgeItem(lenght_, size_, side, brItem);
    //            //                 App::graphicsView()->scene()->addItem(brItem);
    //            //                brItem->setPos(pos);
    //            //                brItem->lastPos_ = pos;
    //            //            }
    //            updateBridge();
    //            brItem = new GiBridge(lenght_, size_, side, brItem);
    //            //        delete item;
    //        }
    //    }
}

#include "moc_profile_form.cpp"
