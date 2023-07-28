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
#include "profile_form.h"
#include "gcprofile_autogen/include/ui_profileform.h"
#include "profile.h"
#include "ui_profileform.h"

#include "gc_gi_bridge.h"
#include "graphicsview.h"
#include "settings.h"
#include <QMessageBox>
#include <ranges>
#include <unordered_set>

template <>
struct std::hash<QPointF> {
    size_t operator()(QPointF const& p) const noexcept {
        size_t h = (size_t)p.x() ^ (size_t)p.y();
        return h;
    }
};

namespace Profile {

Form::Form(GCode::Plugin* plugin, QWidget* parent)
    : GCode::BaseForm(plugin, new Creator, parent)
    , ui(new Ui::ProfileForm) {
    ui->setupUi(content);
    setWindowTitle(tr("Profile Toolpath"));

    ui->pbAddBridge->setIcon(QIcon::fromTheme("edit-cut"));

    ui->cbxBridgeAlignType->addItems({
        tr("Manually"),                    // Вручную.
        tr("Horizontally"),                // По Горизонтали.
        tr("Vertically"),                  // По Вертикали.
        tr("Horizontally and vertically"), // По Горизонтали и вертикали.
        tr("Through the distance"),        // Через расстояние.
        tr("Evenly around the perimeter"), // Равномерно по периметру.
        tr("Split"),                       // Разделить на Х частей.
    });

    MySettings settings;
    settings.beginGroup("Profile");
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

    // clang-format off
    connect(App::grViewPtr(),     &GraphicsView::mouseMove,      this, &Form::updateBridgePos);
    connect(dsbxDepth,            &DepthForm::valueChanged,      this, &Form::updateBridges);
    connect(leName,               &QLineEdit::textChanged,       this, &Form::onNameTextChanged);
    connect(ui->dsbxBridgeLenght, &QDoubleSpinBox::valueChanged, this, &Form::updateBridges);
    connect(ui->pbAddBridge,      &QPushButton::clicked,         this, &Form::onAddBridgeClicked);
    connect(ui->pbClearBridges,   &QPushButton::clicked,         this, &Form::onAddBridgeClicked);
    connect(ui->rbClimb,          &QRadioButton::clicked,        this, &Form::rb_clicked);
    connect(ui->rbConventional,   &QRadioButton::clicked,        this, &Form::rb_clicked);
    connect(ui->rbInside,         &QRadioButton::clicked,        this, &Form::rb_clicked);
    connect(ui->rbOn,             &QRadioButton::clicked,        this, &Form::rb_clicked);
    connect(ui->rbOutside,        &QRadioButton::clicked,        this, &Form::rb_clicked);
    connect(ui->toolHolder,       &ToolSelectorForm::updateName, this, &Form::updateName);
    connect(ui->cbxTrimming,      &QCheckBox::toggled,           this, [this](bool checked) {
        if(side == GCode::On)
            checked ? trimming_ |= Trimming::Line : trimming_ &= ~Trimming::Line;
        else
            checked ? trimming_ |= Trimming::Corner : trimming_ &= ~Trimming::Corner;
    });
    // clang-format on
}

Form::~Form() {
    MySettings settings;
    settings.beginGroup("Profile");
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

    for (QGraphicsItem* giItem: App::grView().items())
        if (giItem->type() == Gi::Type::Bridge)
            delete giItem;
    delete ui;
}

void Form::computePaths() {
    usedItems_.clear();
    const auto tool{ui->toolHolder->tool()};
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    auto gcp = getNewGcp();
    if (!gcp)
        return;

    gcp->setConvent(ui->rbConventional->isChecked());
    gcp->setSide(side);
    gcp->tools.push_back(tool);
    gcp->params[GCode::Params::Depth] = dsbxDepth->value();

    gcp->params[Creator::BridgeAlignType] = ui->cbxBridgeAlignType->currentIndex();
    gcp->params[Creator::BridgeValue] = ui->dsbxBridgeValue->value();
    // NOTE reserve   gcp_.params[Creator::BridgeValue2] = ui->dsbxBridgeValue->value();

    if (side == GCode::On)
        gcp->params[Creator::TrimmingOpenPaths] = ui->cbxTrimming->isChecked();
    else
        gcp->params[Creator::TrimmingCorners] = ui->cbxTrimming->isChecked();

    gcp->params[GCode::Params::GrItems].setValue(usedItems_);

    QPolygonF brv;
    for (QGraphicsItem* item: App::grView().items())
        if (item->type() == Gi::Type::Bridge)
            brv.push_back(item->pos());
    if (!brv.isEmpty()) {
        // gcp_.params[GCode::Params::Bridges].fromValue(brv);
        gcp->params[Creator::BridgeLen] = ui->dsbxBridgeLenght->value();
    }

    fileCount = 1;
    emit createToolpath(gcp);
}

void Form::updateName() {
    leName->setText(names[side]);
    updateBridges();
}

void Form::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void Form::showEvent(QShowEvent* event) {
    updatePixmap();
    QWidget::showEvent(event);
}

void Form::onAddBridgeClicked() {
    if (sender() == ui->pbClearBridges) {
        qDeleteAll(App::grView().items<GiBridge>());
        return;
    }

    const double value = ui->dsbxBridgeValue->value();

    auto addHorizontallyVertically = [this, value](BridgeAlign align) {
        auto testAndAdd = [this](QLineF testLineV, QLineF srcline) {
            QPointF intersects;
            if (auto is = testLineV.intersects(srcline, &intersects); is == QLineF::BoundedIntersection) {
                qDebug() << "intersects1" << is << intersects;
                auto brItem = App::grView().addItem<GiBridge>();
                //                brItem->pathHash = pathHash;
                brItem->setPos(intersects); // NOTE need to collidingItems in snapedPos
                brItem->setPos(brItem->snapedPos(intersects));
                brItem->setVisible(true);
                brItem->setOpacity(1.0);
                if (!brItem->ok())
                    delete brItem;
            }
        };

        for (Gi::Item* gi: App::grView().selectedItems<Gi::Item>()) {
            auto bounds = Bounds(gi->paths());
            int stepH = bounds.Width() / (value + 1);
            int stepV = bounds.Height() / (value + 1);
            for (int var: std::views::iota(1, lround(value) + 1)) {
                QLineF testLineH{
                    Point(bounds.left + stepH * var, bounds.bottom + uScale),
                    Point(bounds.left + stepH * var, bounds.top - uScale)};
                QLineF testLineV{
                    Point(bounds.left - uScale, bounds.top + stepV * var),
                    Point(bounds.right + uScale, bounds.top + stepV * var)};
                for (auto&& path: gi->paths()) {
                    auto pathHash = path.hash();
                    for (int i{}; i < path.size(); ++i) {

                        QLineF srcline{path[i], path[(i + 1) % path.size()]};
                        if (align & Horizontally)
                            testAndAdd(testLineH, srcline);
                        if (align & Vertically)
                            testAndAdd(testLineV, srcline);
                    }
                }
            }
        }
    };

    auto at = BridgeAlign(ui->cbxBridgeAlignType->currentIndex());
    switch (at) {
    case Manually: {
        //        GiBridge::lenght = ui->dsbxBridgeLenght->value();
        //        GiBridge::toolDiam = ui->toolHolder->tool().getDiameter(dsbxDepth->value());
        auto brItem = new GiBridge;
        App::grView().addItem(brItem);
        brItem->setVisible(true);
        brItem->setOpacity(1.0);
        GiBridge::moveBrPtr = brItem;
    } break;
    case Horizontally:
    case Vertically:
    case HorizontallyVertically:
        // qDeleteAll(App::grView().items<GiBridge>());
        addHorizontallyVertically(at);
        break;
    case ThroughTheDistance: {
    } break;
    case EvenlyDround: {
    } break;
    case Split: {
        // qDeleteAll(App::grView().items<GiBridge>());
        std::unordered_set<QPointF> set;
        for (Gi::Item* gi: App::grView().selectedItems<Gi::Item>()) {
            for (auto&& path: gi->paths()) {
                if (path.size() != 2) continue;
                QLineF srcline{path.front(), path.back()};
                if (!set.emplace(srcline.center()).second) continue;
                auto brItem = App::grView().addItem<GiBridge>();
                brItem->setPos(srcline.center()); // NOTE need to collidingItems in snapedPos
                brItem->setPos(brItem->snapedPos(srcline.center()));
                brItem->setVisible(true);
                brItem->setOpacity(1.0);
                if (!brItem->ok())
                    delete brItem;
            }
        }
    } break;
    default:
        break;
    }
}

void Form::updateBridges() {
    GiBridge::lenght = ui->dsbxBridgeLenght->value();
    GiBridge::toolDiam = ui->toolHolder->tool().getDiameter(dsbxDepth->value());
    GiBridge::side = side;
    for (GiBridge* item: App::grView().items<GiBridge>())
        item->update();
}

void Form::updatePixmap() {
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[side + direction * 3]).pixmap(QSize(size, size)));
}

void Form::rb_clicked() {
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

void Form::updateBridgePos(QPointF pos) {
    if (GiBridge::moveBrPtr)
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
    //            //                 App::grView().addItem(brItem);
    //            //                brItem->setPos(pos);
    //            //                brItem->lastPos_ = pos;
    //            //            }
    //            updateBridge();
    //            brItem = new GiBridge(lenght_, size_, side, brItem);
    //            //        delete item;
    //        }
    //    }
}

} // namespace Profile

#include "moc_profile_form.cpp"
