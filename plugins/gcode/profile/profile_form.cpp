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
#include <ranges>

ProfileForm::ProfileForm(GCodePlugin* plugin, QWidget* parent)
    : GcFormBase(plugin, new GCode::ProfileCtr, parent)
    , ui(new Ui::ProfileForm) {
    ui->setupUi(content);
    setWindowTitle(tr("Profile Toolpath"));

    ui->pbAddBridge->setIcon(QIcon::fromTheme("edit-cut"));

    ui->cbxBridgeAlignType->addItems({
        /*Вручную. По Горизонтали. По Вертикали. По Горизонтали и вертикали. Через расстояние. Равномерно по периметру.*/
        tr("Manually"),
        tr("Horizontally"),
        tr("Vertically"),
        tr("Horizontally and vertically"),
        tr("Through the distance"),
        tr("Evenly around the perimeter"),
    });

    MySettings settings;
    settings.beginGroup("ProfileForm");
    settings.getValue(ui->dsbxBridgeLenght, 1.0);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOn);
    settings.getValue(ui->rbOutside);
    settings.getValue(ui->cbxTrimming);
    settings.getValue(ui->dsbxBridgeValue, 1.0);
    settings.getValue(ui->cbxBridgeAlignType, 1.0);
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
    settings.setValue(ui->cbxBridgeAlignType);
    settings.setValue(ui->dsbxBridgeValue);
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

    gcp_.params[GCode::ProfileCtr::BridgeAlignType] = ui->cbxBridgeAlignType->currentIndex();
    gcp_.params[GCode::ProfileCtr::BridgeValue] = ui->dsbxBridgeValue->value();
    // NOTE reserve   gcp_.params[GCode::ProfileCtr::BridgeValue2] = ui->dsbxBridgeValue->value();

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
    static auto solid = [](auto* item) { return item->type() == GiType::DataSolid; };

    const double value = ui->dsbxBridgeValue->value();

    switch (ui->cbxBridgeAlignType->currentIndex()) {
    case Manually: {
        if (brItem) {
            if (!brItem->ok())
                delete brItem;
        }
        brItem = new GiBridge(lenght_, size_, side, brItem);
        App::graphicsView()->scene()->addItem(brItem);
        brItem->setVisible(true);
        brItem->setOpacity(1.0);
    } break;
    case Horizontally: {
        for (auto* item : App::graphicsView()->scene()->selectedItems() /* | std::views::filter(solid)*/) {
            GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);

            if (gi->type() == GiType::DataSolid) {

                auto bounds = Bounds(gi->paths());
                // dbgPaths(gi->paths(), __FUNCTION__, Qt::magenta);

                int step = bounds.Width() / (value + 1);
                if (0) {
                    Clipper clipper;
                    for (int var : std::views::iota(1, lround(value) + 1)) {
                        Path p {
                            {bounds.left + step * var, bounds.bottom + uScale},
                            {bounds.left + step * var,    bounds.top - uScale}
                        };
                        //                    dbgPaths({p}, __FUNCTION__, Qt::red);
                        clipper.AddOpenSubject({p});
                    }

                    clipper.AddClip(gi->paths());
                    Paths result;
                    Paths _;
                    clipper.Execute(CT::Intersection, FR::/*NonZero*/ EvenOdd, _, result);
                }

                for (int var : std::views::iota(1, lround(value) + 1)) {
                    QLineF testLine {
                        Point(bounds.left + step * var, bounds.bottom + uScale),
                        Point(bounds.left + step * var, bounds.top - uScale)};

                    for (auto&& path : gi->paths()) {
                        for (int i {}; i < path.size(); ++i) {
                            QLineF srcline {path[i], path[(i + 1) % path.size()]};
                            QPointF intersects;
                            if (auto is = testLine.intersects(srcline, &intersects); is == QLineF::BoundedIntersection) {
                                qDebug() << "intersects1" << is << intersects;
                                brItem = new GiBridge(lenght_, size_, side, brItem);
                                App::graphicsView()->scene()->addItem(brItem);
                                brItem->setPos(intersects);
                                // qDebug() << __FUNCTION__ << brItem->calculate(intersects) << intersects;
                                brItem->setOk(true);
                                if (!brItem->ok())
                                    delete brItem;
                                else {
                                    brItem->setVisible(true);
                                    brItem->setOpacity(1.0);
                                }
                            }
                        }
                    }
                }
            }
        }
    } break;
    case Vertically: {
    } break;
    case HorizontallyVertically: {
    } break;
    case ThroughTheDistance: {
    } break;
    case EvenlyDround: {
    } break;
    default:
        break;
    }
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
