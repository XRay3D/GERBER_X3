// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thread_form.h"

#include "gc_gi_bridge.h"
#include "graphicsview.h"
#include "settings.h"
#include "ui_threadform.h"
#include <QMessageBox>
#include <ranges>

namespace Thread {

Form::Form(GCode::Plugin* plugin, QWidget* parent)
    : GCode::BaseForm(plugin, new Creator, parent)
    , ui(new Ui::ThreadForm) {
    ui->setupUi(content);
    setWindowTitle(tr("Thread Toolpath"));

    //    MySettings settings;
    //    settings.beginGroup("Form");
    //    settings.getValue(ui->dsbxBridgeLenght, 1.0);
    //    settings.getValue(ui->rbClimb);
    //    settings.getValue(ui->rbConventional);
    //    settings.getValue(ui->rbInside);
    //    settings.getValue(ui->rbOn);
    //    settings.getValue(ui->rbOutside);
    //    settings.getValue(ui->cbxTrimming);
    //    settings.getValue(ui->dsbxBridgeValue, 1.0);
    //    settings.getValue(ui->cbxBridgeAlignType, 1.0);
    //    settings.getValue(varName(trimming_), 0);
    //    settings.endGroup();

    //    rb_clicked();

    //    // clang-format off
    //    connect(App::graphicsViewPtr(), &GraphicsView::mouseMove,      this, &Form::updateBridgePos);
    //    connect(dsbxDepth,              &DepthForm::valueChanged,      this, &Form::updateBridges);
    //    connect(leName,                 &QLineEdit::textChanged,       this, &Form::onNameTextChanged);
    //    connect(ui->dsbxBridgeLenght,   &QDoubleSpinBox::valueChanged, this, &Form::updateBridges);
    //    connect(ui->pbAddBridge,        &QPushButton::clicked,         this, &Form::onAddBridgeClicked);
    //    connect(ui->rbClimb,            &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->rbConventional,     &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->rbInside,           &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->rbOn,               &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->rbOutside,          &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->toolHolder,         &ToolSelectorForm::updateName, this, &Form::updateName);
    //    // clang-format on

    //    connect(ui->cbxTrimming, &QCheckBox::toggled, [this](bool checked) {
    //        if (side == GCode::On)
    //            checked ? trimming_ |= Trimming::Line : trimming_ &= ~Trimming::Line;
    //        else
    //            checked ? trimming_ |= Trimming::Corner : trimming_ &= ~Trimming::Corner;
    //    });
}

Form::~Form() {
    //    MySettings settings;
    //    settings.beginGroup("Form");
    //    settings.setValue(ui->dsbxBridgeLenght);
    //    settings.setValue(ui->rbClimb);
    //    settings.setValue(ui->rbConventional);
    //    settings.setValue(ui->rbInside);
    //    settings.setValue(ui->rbOn);
    //    settings.setValue(ui->rbOutside);
    //    settings.setValue(ui->cbxTrimming);
    //    settings.setValue(ui->cbxBridgeAlignType);
    //    settings.setValue(ui->dsbxBridgeValue);
    //    settings.setValue(varName(trimming_));
    //    settings.endGroup();

    //    for (QGraphicsItem* giItem : App::graphicsView().items()) {
    //        if (giItem->type() == Gi::Type::Bridge)
    //            delete giItem;
    //    }
    delete ui;
}

void Form::computePaths() {
    //    usedItems_.clear();
    //    const auto tool {ui->toolHolder->tool()};
    //    if (!tool.isValid()) {
    //        tool.errorMessageBox(this);
    //        return;
    //    }

    //    Paths wPaths;
    //    Paths wRawPaths;
    //    AbstractFile const* file = nullptr;
    //    bool skip {true};

    //    for (auto* gi : App::graphicsView().selectedItems<Gi::Item>()) {
    //        switch (gi->type()) {
    //        case Gi::Type::DataSolid:
    //            wPaths.append(gi->paths());
    //            break;
    //        case Gi::Type::DataPath: {
    //            auto paths = gi->paths();
    //            if (paths.front() == paths.back())
    //                wPaths.append(paths);
    //            else
    //                wRawPaths.append(paths);
    //        } break;
    //            //            if (!file) {
    //            //                file = gi->file();
    //            //                boardSide = file->side();
    //            //            } else if (file != gi->file()) {
    //            //                if (skip) {
    //            //                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
    //            //                        return;
    //            //                }
    //            //            }
    //            //            if (gi->type() == Gi::Type::DataSolid)
    //            //                wPaths.append(gi->paths());
    //            //            else
    //            //                wRawPaths.append(gi->paths());
    //            //            break;
    //        case Gi::Type::ShCircle:
    //        case Gi::Type::ShRectangle:
    //        case Gi::Type::ShText:
    //        case Gi::Type::Drill:
    //            wPaths.append(gi->paths());
    //            break;
    //        case Gi::Type::ShPolyLine:
    //        case Gi::Type::ShCirArc:
    //            wRawPaths.append(gi->paths());
    //            break;
    //        default:
    //            break;
    //        }
    //        addUsedGi(gi);
    //    }

    //    if (wRawPaths.empty() && wPaths.empty()) {
    //        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
    //        return;
    //    }

    //    auto gcp = new GCode::Params;
    //    gcp->setConvent(ui->rbConventional->isChecked());
    //    gcp->setSide(side);
    //    gcp->tools.push_back(tool);
    //    gcp->params[GCode::Params::Depth] = dsbxDepth->value();

    //    gcp->params[Creator::BridgeAlignType] = ui->cbxBridgeAlignType->currentIndex();
    //    gcp->params[Creator::BridgeValue] = ui->dsbxBridgeValue->value();
    //    // NOTE reserve   gcp_.params[Creator::BridgeValue2] = ui->dsbxBridgeValue->value();

    //    if (side == GCode::On)
    //        gcp->params[Creator::TrimmingOpenPaths] = ui->cbxTrimming->isChecked();
    //    else
    //        gcp->params[Creator::TrimmingCorners] = ui->cbxTrimming->isChecked();

    //    gcp->params[GCode::Params::GrItems].setValue(usedItems_);

    //    QPolygonF brv;
    //    for (QGraphicsItem* item : App::graphicsView().items()) {
    //        if (item->type() == Gi::Type::Bridge)
    //            brv.push_back(item->pos());
    //    }
    //    if (!brv.isEmpty()) {
    //        // gcp_.params[GCode::Params::Bridges].fromValue(brv);
    //        gcp->params[Creator::BridgeLen] = ui->dsbxBridgeLenght->value();
    //    }

    //    gcp->closedPaths = std::move(wPaths);
    //    gcp->openPaths = std::move(wRawPaths);
    //    fileCount = 1;
    //    emit createToolpath(gcp);
}

void Form::updateName() {
    leName->setText(names[side]);
}

void Form::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void Form::showEvent(QShowEvent* event) {
    updatePixmap();
    QWidget::showEvent(event);
}

void Form::updatePixmap() {
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[side + direction * 3]).pixmap(QSize(size, size)));
}

void Form::rb_clicked() {
    //    if (ui->rbOn->isChecked()) {
    //        side = GCode::On;
    //        ui->cbxTrimming->setText(tr("Trimming"));
    //        ui->cbxTrimming->setChecked(trimming_ & Trimming::Line);
    //    } else if (ui->rbOutside->isChecked()) {
    //        side = GCode::Outer;
    //        ui->cbxTrimming->setText(tr("Corner Trimming"));
    //        ui->cbxTrimming->setChecked(trimming_ & Trimming::Corner);
    //    } else if (ui->rbInside->isChecked()) {
    //        side = GCode::Inner;
    //        ui->cbxTrimming->setText(tr("Corner Trimming"));
    //        ui->cbxTrimming->setChecked(trimming_ & Trimming::Corner);
    //    }

    //    if (ui->rbClimb->isChecked())
    //        direction = GCode::Climb;
    //    else if (ui->rbConventional->isChecked())
    //        direction = GCode::Conventional;

    //    updateName();
    //    updateButtonIconSize();

    //    updatePixmap();
}

void Form::updateBridgePos(QPointF pos) {
    if(GiBridge::moveBrPtr)
        GiBridge::moveBrPtr->setPos(pos);
}

void Form::onNameTextChanged(const QString& arg1) { fileName_ = arg1; }

void Form::editFile(GCode::File* file) {

    //    GCode::Params gcp_ {file->gcp()};

    //    fileId = gcp_.fileId;
    //    editMode_ = true;

    //    { // GUI
    //        side = gcp_.side();
    //        direction = static_cast<GCode::Direction>(gcp_.convent());
    //        ui->toolHolder->setTool(gcp_.tools.front());
    //        dsbxDepth->setValue(gcp_.params[GCode::Params::Depth].toDouble());

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
    //        auto items {gcp_.params[GCode::Params::GrItems].value<UsedItems>()};

    //        auto i = items.cbegin();
    //        while (i != items.cend()) {

    //            //            auto [_fileId, _] = i.key();
    //            //            Q_UNUSED(_)
    //            //            App::project().file(_fileId)->itemGroup()->setSelected(i.value());
    //            //            ++i;
    //        }
    //    }

    //    { // Bridges
    //        if (gcp_.params.contains(GCode::Params::Bridges)) {
    //            ui->dsbxBridgeLenght->setValue(gcp_.params[GCode::Params::BridgeLen].toDouble());
    //            //            for (auto& pos : gcp_.params[GCode::Params::Bridges].value<QPolygonF>()) {
    //            //                brItem = new BridgeItem(lenght_, size_, side, brItem);
    //            //                 App::graphicsView().addItem(brItem);
    //            //                brItem->setPos(pos);
    //            //                brItem->lastPos_ = pos;
    //            //            }
    //            updateBridge();
    //            brItem = new GiBridge(lenght_, size_, side, brItem);
    //            //        delete item;
    //        }
    //    }
}

} // namespace Thread

#include "moc_thread_form.cpp"
