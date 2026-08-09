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

Form::Form(GCode::Plugin* plugin)
    : GCode::Form{plugin, new Creator}
    , ui{new Ui::ProfileForm} {
    ui->setupUi(content);
    setWindowTitle(tr("Profile Toolpath"));

    ui->pbAddBridge->setIcon(QIcon::fromTheme(u"edit-cut"_s));

    // Мосты отключены до отдельного захода: Creator::makeBridges режет ОТКРЫТЫЙ
    // путь замкнутой областью, а такого примитива в Geo нет (у Clipper2 он был
    // -- Clipper64::AddOpenSubject + Difference). Ставить мосты, которые потом
    // никак не попадут в траекторию, хуже, чем не давать их ставить вовсе.
    ui->groupBox_3->setEnabled(false); // группа «Bridges»

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
    settings.beginGroup(u"Profile"_s);
    settings.getValue(ui->dsbxBridgeLenght, 1.0);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOn);
    settings.getValue(ui->rbOutside);
    settings.getValue(ui->cbxTrimming);
    settings.getValue(ui->dsbxBridgeValue, 1.0);
    settings.getValue(ui->cbxBridgeAlignType, 1.0);
    settings.getValue(ui->dsbxAllowance, 0.0);
    settings.getValue(ui->cbxSpiral, true);
    settings.getValue(varName(trimming_), 0);
    settings.endGroup();

    updateAllowanceLimits();

    rb_clicked();

    // clang-format off
    connect(App::grViewPtr(),     &GraphicsView::mouseMove,      this, &Form::updateBridgePos);
    connect(dsbxDepth,            &DepthForm::valueChanged,      this, &Form::updateBridges);
    connect(dsbxDepth,            &DepthForm::valueChanged,      this, &Form::updateAllowanceLimits);
    connect(ui->toolHolder,       &ToolSelectorForm::updateName, this, &Form::updateAllowanceLimits);
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
    settings.beginGroup(u"Profile"_s);
    settings.setValue(ui->dsbxBridgeLenght);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbOn);
    settings.setValue(ui->rbOutside);
    settings.setValue(ui->cbxTrimming);
    settings.setValue(ui->cbxBridgeAlignType);
    settings.setValue(ui->dsbxBridgeValue);
    settings.setValue(ui->dsbxAllowance);
    settings.setValue(ui->cbxSpiral);
    settings.setValue(varName(trimming_));
    settings.endGroup();

    for(QGraphicsItem* giItem: App::grView().items())
        if(giItem->type() == Gi::Type::Bridge)
            delete giItem;
    delete ui;
}

void Form::computePaths() {

    const auto tool{ui->toolHolder->tool()};
    if(!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    if(!getNewGcpWithGi()) return;

    gcp.setConvent(ui->rbConventional->isChecked());
    gcp.setSide(side);
    gcp.tools.push_back(tool);
    gcp.params[GCode::Params::Depth] = dsbxDepth->value();

    gcp.params[GCode::Params::SpiralRamp] = ui->cbxSpiral->isChecked();

    if(side == GCode::On)
        gcp.params[Creator::TrimmingOpenPaths] = ui->cbxTrimming->isChecked();
    else {
        gcp.params[Creator::TrimmingCorners] = ui->cbxTrimming->isChecked();
        // При On офсета нет вовсе -- припуску там негде жить, виджет погашен.
        gcp.params[Creator::Allowance] = ui->dsbxAllowance->value();
    }

    // BridgeAlignType/BridgeValue/BridgeLen не кладутся: Creator их всё равно
    // не читает, пока makeBridges отключён (см. конструктор).

    fileCount = 1;
    emit createToolpath(&gcp);
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
    if(sender() == ui->pbClearBridges) {
        qDeleteAll(App::grView().items<Gi::Bridge>());
        return;
    }

    const double value = ui->dsbxBridgeValue->value();

    auto addHorizontallyVertically = [this, value](BridgeAlign align) {
        auto testAndAdd = [this](QLineF testLineV, QLineF srcline) {
            QPointF intersects;
            if(auto is = testLineV.intersects(srcline, &intersects); is == QLineF::BoundedIntersection) {
                qDebug() << u"intersects1"_s << is << intersects;
                auto brItem = App::grView().addItem<Gi::Bridge>();
                // brItem->pathHash = pathHash;
                brItem->setPos(intersects); // NOTE need to collidingItems in snapedPos
                brItem->setPos(brItem->snapedPos(intersects));
                brItem->setVisible(true);
                brItem->setOpacity(1.0);
                if(!brItem->ok())
                    delete brItem;
            }
        };

#if 0 // FIXME

        for(Gi::Item* gi: App::grView().selectedItems<Gi::Item>()) {
            auto bounds = BoundingRect(gi->curves());
            int stepH = bounds.width() / (value + 1);
            int stepV = bounds.height() / (value + 1);
            for(int var: v::iota(1, lround(value) + 1)) {
                QLineF testLineH{
                    {bounds.left() + stepH * var, bounds.bottom() + 1},
                    {bounds.left() + stepH * var, bounds.top() - 1   }
                };
                QLineF testLineV{
                    {bounds.left() - 1,  bounds.top() + stepV * var},
                    {bounds.right() + 1, bounds.top() + stepV * var}
                };
                for(auto&& curve: gi->curves()) {
                    // auto pathHash = path.hash();
                    for(auto [fr, to]: curve | v::pairwise) {
                        QLineF srcline{fr, to};
                        if(align & Horizontally) testAndAdd(testLineH, srcline);
                        if(align & Vertically) testAndAdd(testLineV, srcline);
                    }
                    // for(size_t i{}; i < path.size(); ++i) {
                    //     QLineF srcline{~path[i], ~path[(i + 1) % path.size()]};
                    //     if(align & Horizontally) testAndAdd(testLineH, srcline);
                    //     if(align & Vertically) testAndAdd(testLineV, srcline);
                    // }
                }
            }
        }
#endif
    };

    auto at = BridgeAlign(ui->cbxBridgeAlignType->currentIndex());
    switch(at) {
    case Manually: {
        // Gi::Bridge::lenght = ui->dsbxBridgeLenght->value();
        // Gi::Bridge::toolDiam = ui->toolHolder->tool().getDiameter(dsbxDepth->value());
        auto brItem = new Gi::Bridge;
        App::grView().addItem(brItem);
        brItem->setVisible(true);
        brItem->setOpacity(1.0);
        Gi::Bridge::moveBrPtr = brItem;
    } break;
    case Horizontally:
    case Vertically:
    case HorizontallyVertically:
        // qDeleteAll(App::grView().items<Gi::Bridge>());
        addHorizontallyVertically(at);
        break;
    case ThroughTheDistance: {
    } break;
    case EvenlyDround: {
    } break;
    case Split: {
        // qDeleteAll(App::grView().items<Gi::Bridge>());
        std::unordered_set<QPointF> set;
        for(Gi::Item* gi: App::grView().selectedItems<Gi::Item>()) {
            for(auto&& curve: gi->curves()) {
                if(curve.size() != 2) continue;
                QLineF srcline{curve.front(), curve.back()};
                if(!set.emplace(srcline.center()).second) continue;
                auto brItem = App::grView().addItem<Gi::Bridge>();
                brItem->setPos(srcline.center()); // NOTE need to collidingItems in snapedPos
                brItem->setPos(brItem->snapedPos(srcline.center()));
                brItem->setVisible(true);
                brItem->setOpacity(1.0);
                if(!brItem->ok())
                    delete brItem;
            }
        }
    } break;
    default: break;
    }
}

void Form::updateBridges() {
    Gi::Bridge::lenght   = ui->dsbxBridgeLenght->value();
    Gi::Bridge::toolDiam = ui->toolHolder->tool().getDiameter(dsbxDepth->value());
    Gi::Bridge::side     = side;
    for(Gi::Bridge* item: App::grView().items<Gi::Bridge>())
        item->update();
}

void Form::updatePixmap() {
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[side + direction * 3]).pixmap(QSize(size, size)));
}

void Form::rb_clicked() {
    if(ui->rbOn->isChecked()) {
        side = GCode::On;
        ui->cbxTrimming->setText(tr("Trimming"));
        ui->cbxTrimming->setChecked(trimming_ & Trimming::Line);
    } else if(ui->rbOutside->isChecked()) {
        side = GCode::Outer;
        ui->cbxTrimming->setText(tr("Corner Trimming"));
        ui->cbxTrimming->setChecked(trimming_ & Trimming::Corner);
    } else if(ui->rbInside->isChecked()) {
        side = GCode::Inner;
        ui->cbxTrimming->setText(tr("Corner Trimming"));
        ui->cbxTrimming->setChecked(trimming_ & Trimming::Corner);
    }

    if(ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if(ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    updateName();
    updateButtonIconSize();
    updateAllowanceLimits();

    updatePixmap();
}

void Form::updateAllowanceLimits() {
    // При On контур не смещается вовсе, чистовому проходу неоткуда взяться.
    ui->dsbxAllowance->setEnabled(side != GCode::On);

    const double radius = ui->toolHolder->tool().getDiameter(dsbxDepth->value()) * 0.5;
    if(radius <= 0.0) return;

    // Больше половины радиуса за один чистовой проход не снять -- дальше это уже
    // не зачистка, а второй черновой.
    ui->dsbxAllowance->setMaximum(radius * 0.5);

    // Разумная величина -- десятая радиуса; всё, что больше, приводим к ней при
    // смене инструмента или глубины. Ноль не трогаем: это «чистового не надо».
    const double sane = radius * 0.1;
    if(!qFuzzyIsNull(ui->dsbxAllowance->value()) && ui->dsbxAllowance->value() > sane)
        ui->dsbxAllowance->setValue(sane);
}

void Form::updateBridgePos(QPointF pos) {
    if(Gi::Bridge::moveBrPtr)
        Gi::Bridge::moveBrPtr->setPos(pos);
}

void Form::onNameTextChanged(const QString& arg1) { fileName_ = arg1; }

void Form::editFile(GCode::File* file) {
    const GCode::Params& gcp = file->params();

    // Радиокнопки первыми: их rb_clicked() зовёт updateName() и updateAllowanceLimits(),
    // то есть перебивает и имя, и припуск. База ставит имя после этого.
    if(gcp.params.contains(GCode::Params::Side))
        switch(gcp.side()) {
        case GCode::On   : ui->rbOn->setChecked(true); break;
        case GCode::Outer: ui->rbOutside->setChecked(true); break;
        case GCode::Inner: ui->rbInside->setChecked(true); break;
        }
    if(gcp.params.contains(GCode::Params::Convent))
        (gcp.convent() ? ui->rbConventional : ui->rbClimb)->setChecked(true);

    // Инструмент -- копия, снятая при расчёте: в базе инструментов его могло уже
    // не остаться, поэтому ставим как есть, не разыскивая по id.
    if(!gcp.tools.empty()) ui->toolHolder->setTool(gcp.tool());

    ui->cbxSpiral->setChecked(gcp.spiralRamp());

    auto value = [&gcp](const QString& key) {
        auto it = gcp.params.find(key);
        return it == gcp.params.end() ? GCode::Variant{} : it->second;
    };

    ui->cbxTrimming->setChecked(gcp.params.contains(Creator::TrimmingOpenPaths)
            ? value(Creator::TrimmingOpenPaths).toBool()
            : value(Creator::TrimmingCorners).toBool());
    ui->dsbxAllowance->setValue(value(Creator::Allowance).toDouble());

    // База ставит глубину, имя, сторону платы и восстанавливает сцену. Припуск
    // после неё уже не тронется: updateAllowanceLimits() зовут только виджеты.
    GCode::Form::editFile(file);
}

} // namespace Profile

#include "moc_profile_form.cpp"
