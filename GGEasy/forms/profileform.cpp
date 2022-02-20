// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "profileform.h"
#include "ui_profileform.h"

#include "bridgeitem.h"
#include "project.h"
#include "scene.h"
#include "settings.h"
#include <QMessageBox>

ProfileForm::ProfileForm(QWidget* parent)
    : FormsUtil(new GCode::ProfileCreator, parent)
    , ui(new Ui::ProfileForm)
    , names { tr("Profile On"), tr("Profile Outside"), tr("Profile Inside") }
//    , pixmaps {
//        "prof_on_climb",
//        "prof_out_climb",
//        "prof_in_climb",
//        "prof_on_conv",
//        "prof_out_conv",
//        "prof_in_conv",
//    }
{
    ui->setupUi(this);
    parent->setWindowTitle(ui->label->text());

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));
    ui->pbAddBridge->setIcon(QIcon::fromTheme("edit-cut"));

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({ 16, 16 });

    MySettings settings;
    settings.beginGroup("ProfileForm");
    settings.getValue(ui->dsbxBridgeLenght, 1.0);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOn);
    settings.getValue(ui->rbOutside);
    settings.getValue(ui->cbxTrimming);
    settings.getValue(varName(m_trimming), 0);
    settings.endGroup();

    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbOn, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);

    connect(ui->dsbxBridgeLenght, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ProfileForm::updateBridge);
    connect(ui->dsbxDepth, &DepthForm::valueChanged, this, &ProfileForm::updateBridge);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &ProfileForm::updateName);

    connect(ui->pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);
    connect(ui->pbCreate, &QPushButton::clicked, this, &ProfileForm::createFile);
    connect(ui->cbxTrimming, &QCheckBox::toggled, [this](bool checked) {
        if (side == GCode::On)
            checked ? m_trimming |= Trimming::Line
                    : m_trimming &= ~Trimming::Line;
        else
            checked ? m_trimming |= Trimming::Corner
                    : m_trimming &= ~Trimming::Corner;
    });
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
    settings.setValue(varName(m_trimming));
    settings.endGroup();

    for (QGraphicsItem* giItem : App::scene()->items()) {
        if (static_cast<GiType>(giItem->type()) == GiType::Bridge)
            delete giItem;
    }
    delete ui;
}

void ProfileForm::createFile() {
    m_usedItems.clear();
    const auto tool { ui->toolHolder->tool() };
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    FileInterface const* file = nullptr;
    bool skip { true };

    for (auto* sItem : App::scene()->selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(sItem);
        switch (static_cast<GiType>(sItem->type())) {
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
            if (static_cast<GiType>(sItem->type()) == GiType::DataSolid)
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

    GCode::GCodeParams gcp;
    gcp.setConvent(ui->rbConventional->isChecked());
    gcp.setSide(side);
    gcp.tools.push_back(tool);
    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
    (side == GCode::On) ? gcp.params[GCode::GCodeParams::Trimming] = ui->cbxTrimming->isChecked()
                        : gcp.params[GCode::GCodeParams::CornerTrimming] = ui->cbxTrimming->isChecked();
    gcp.params[GCode::GCodeParams::GrItems].setValue(m_usedItems);

    {
        QPolygonF brv;
        for (QGraphicsItem* item : App::scene()->items()) {
            if (static_cast<GiType>(item->type()) == GiType::Bridge)
                brv.push_back(item->pos());
        }
        if (!brv.isEmpty()) {
            //gcp.params[GCode::GCodeParams::Bridges].fromValue(brv);
            gcp.params[GCode::GCodeParams::BridgeLen] = ui->dsbxBridgeLenght->value();
        }
    }

    m_tpc->setGcp(gcp);
    m_tpc->addPaths(wPaths);
    m_tpc->addRawPaths(wRawPaths);
    fileCount = 1;
    emit createToolpath();
}

void ProfileForm::updateName() {
    ui->leName->setText(names[side]);
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

void ProfileForm::on_pbAddBridge_clicked() {
    if (brItem) {
        if (!brItem->ok())
            delete brItem;
    }
    brItem = new BridgeItem(m_lenght, m_size, side, brItem);
    App::scene()->addItem(brItem);
    brItem->setVisible(true);
}

void ProfileForm::updateBridge() {
    m_lenght = ui->dsbxBridgeLenght->value();
    m_size = ui->toolHolder->tool().getDiameter(ui->dsbxDepth->value());
    for (QGraphicsItem* item : App::scene()->items()) {
        if (static_cast<GiType>(item->type()) == GiType::Bridge)
            static_cast<BridgeItem*>(item)->update();
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
        ui->cbxTrimming->setChecked(m_trimming & Trimming::Line);
    } else if (ui->rbOutside->isChecked()) {
        side = GCode::Outer;
        ui->cbxTrimming->setText(tr("Corner Trimming"));
        ui->cbxTrimming->setChecked(m_trimming & Trimming::Corner);
    } else if (ui->rbInside->isChecked()) {
        side = GCode::Inner;
        ui->cbxTrimming->setText(tr("Corner Trimming"));
        ui->cbxTrimming->setChecked(m_trimming & Trimming::Corner);
    }

    if (ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if (ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    updateName();
    updatePixmap();
}

void ProfileForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void ProfileForm::editFile(GCode::File* file) {

    GCode::GCodeParams gcp { file->gcp() };

    fileId = gcp.fileId;
    m_editMode = true;

    { // GUI
        side = gcp.side();
        direction = static_cast<GCode::Direction>(gcp.convent());
        ui->toolHolder->setTool(gcp.tools.front());
        ui->dsbxDepth->setValue(gcp.params[GCode::GCodeParams::Depth].toDouble());

        switch (side) {
        case GCode::On:
            ui->rbOn->setChecked(true);
            break;
        case GCode::Outer:
            ui->rbOutside->setChecked(true);
            break;
        case GCode::Inner:
            ui->rbInside->setChecked(true);
            break;
        }

        switch (direction) {
        case GCode::Climb:
            ui->rbClimb->setChecked(true);
            break;
        case GCode::Conventional:
            ui->rbConventional->setChecked(true);
            break;
        }
    }

    { // GrItems
        m_usedItems.clear();
        auto items { gcp.params[GCode::GCodeParams::GrItems].value<UsedItems>() };

        auto i = items.cbegin();
        while (i != items.cend()) {

            //            auto [_fileId, _] = i.key();
            //            Q_UNUSED(_)
            //            App::project()->file(_fileId)->itemGroup()->setSelected(i.value());
            //            ++i;
        }
    }

    { // Bridges
        if (gcp.params.contains(GCode::GCodeParams::Bridges)) {
            ui->dsbxBridgeLenght->setValue(gcp.params[GCode::GCodeParams::BridgeLen].toDouble());
            //            for (auto& pos : gcp.params[GCode::GCodeParams::Bridges].value<QPolygonF>()) {
            //                brItem = new BridgeItem(m_lenght, m_size, side, brItem);
            //                App::scene()->addItem(brItem);
            //                brItem->setPos(pos);
            //                brItem->m_lastPos = pos;
            //            }
            updateBridge();
            brItem = new BridgeItem(m_lenght, m_size, side, brItem);
            //        delete item;
        }
    }
}
