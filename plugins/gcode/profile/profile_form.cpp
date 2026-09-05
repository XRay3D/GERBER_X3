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
#include "geo/util.h"
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
    settings.getValue(ui->dsbxBridgeHeight, 0.5);
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
    settings.setValue(ui->dsbxBridgeHeight);
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

    // На выходе из приложения вью уже разрушено вместе с мостиками.
    if(auto* view = App::grViewPtr())
        for(QGraphicsItem* giItem: view->items())
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

    // Мостики: в расчёт уходят центры ok-мостов полилинией в supportCurvess
    // (сериализуется с проектом), длина и высота таба -- параметрами. Круги
    // реза по центрам строит File::saveMillingProfileBridges: их радиус
    // зависит от припуска, известного только там.
    updateBridges(); // статики моста -- к текущему инструменту и стороне
    Geo::Polyline bridgePts;
    for(Gi::Bridge* item: App::grView().items<Gi::Bridge>())
        if(item->ok()) bridgePts.emplace_back(item->pos());
    if(!bridgePts.empty()) {
        Geo::Polylines group;
        group.emplace_back(std::move(bridgePts));
        gcp.supportCurvess.emplace_back(std::move(group));
        gcp.params[Creator::BridgeLen] = ui->dsbxBridgeLenght->value();
        gcp.params[Creator::BridgeHeight] = ui->dsbxBridgeHeight->value();
    }

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
    // Базовая, а не QWidget: в GCode::Form::showEvent сбрасывается режим правки.
    GCode::Form::showEvent(event);
}

void Form::onAddBridgeClicked() {
    if(sender() == ui->pbClearBridges) {
        qDeleteAll(App::grView().items<Gi::Bridge>());
        return;
    }

    updateBridges(); // статики моста -- к текущему инструменту и стороне

    const double value = ui->dsbxBridgeValue->value();
    // Место, которое мост занимает на контуре: перемычка плюс след фрезы.
    const double footprint = Gi::Bridge::lenght() + Gi::Bridge::toolDiam();

    std::unordered_set<QPointF> used; // не сажать два моста в одну точку

    // Мост в точке: добавить, прилипить, оставить -- если прилип.
    auto addAt = [&used](QPointF pos) {
        if(!used.emplace(pos).second) return;
        auto* brItem = App::grView().addItem<Gi::Bridge>();
        brItem->setVisible(true);
        brItem->setOpacity(1.0);
        brItem->setPos(pos); // NOTE сперва встать: snapedPos ищет collidingItems
        brItem->setPos(brItem->snapedPos(pos));
        if(!brItem->ok())
            delete brItem;
    };

    // Предпочтительные посадочные места -- СЕРЕДИНЫ прямых сегментов, где
    // мост помещается целиком: на прямой перемычка выходит ровной, а у дуги
    // и коротышки шансов меньше -- они на крайний случай.
    struct Candidate {
        double at;     // положение середины по длине дуги контура
        QPointF point; // сама середина
        bool used{};
    };
    auto candidatesOf = [footprint](const Geo::Polyline& curve) {
        std::vector<Candidate> candidates;
        double at{};
        for(auto&& [fr, to]: Geo::segments(curve)) {
            const double segLen = Geo::segmentLength(fr, to);
            if(!fr.isArc() && segLen >= footprint)
                candidates.push_back({at + segLen * 0.5, (fr + to) * 0.5});
            at += segLen;
        }
        return candidates;
    };

    // Точка на расстоянии length вдоль контура (по дуге, не по хорде).
    auto pointAtLength = [](const Geo::Polyline& curve, double length) {
        for(auto&& [fr, to]: Geo::segments(curve)) {
            const double segLen = Geo::segmentLength(fr, to);
            if(length > segLen) {
                length -= segLen;
                continue;
            }
            const double t = segLen > 0.0 ? length / segLen : 0.0;
            if(auto arc = Geo::arcOf(fr, to, fr.bulge)) return arc->pointAt(t);
            return QPointF{fr + (to - fr) * t};
        }
        return curve.empty() ? QPointF{} : QPointF{curve.back()};
    };

    // Вдоль контура: либо через каждые value мм, либо value штук на периметр.
    // Каждая цель притягивается к ближайшей свободной середине прямого
    // участка; не дальше полушага -- иначе место уже «чужое», и лучше дуга
    // точно в цели, чем прямая на другом конце контура.
    auto addAlong = [&](bool evenly) {
        for(Gi::Item* gi: App::grView().selectedItems<Gi::Item>())
            for(const Geo::Polyline& curve: gi->curves()) {
                const double perimeter = curve.perimeter();
                if(perimeter <= 0.0) continue;
                const int count = evenly ? int(value) : int(perimeter / value);
                if(count < 1) continue;
                const double step = evenly ? perimeter / count : value;
                auto candidates = candidatesOf(curve);
                for(int i{}; i < count; ++i) {
                    const double target = step * i;
                    Candidate* best{};
                    double bestDist = std::numeric_limits<double>::max();
                    for(Candidate& candidate: candidates) {
                        if(candidate.used) continue;
                        double dist = std::abs(candidate.at - target);
                        if(curve.closed) // по кольцу ближе бывает через шов
                            dist = std::min(dist, perimeter - dist);
                        if(dist < bestDist) bestDist = dist, best = &candidate;
                    }
                    if(best && bestDist <= step * 0.5) {
                        best->used = true;
                        addAt(best->point);
                    } else
                        addAt(pointAtLength(curve, target));
                }
            }
    };

    // Сетка по габариту выделенного: value линий по горизонтали/вертикали,
    // мост -- в пересечении линии с контуром, по возможности сдвинутый к
    // середине прямого сегмента, в который попал. Дуга проверяется хордой:
    // точную посадку на неё доводит snapedPos.
    auto addHorizontallyVertically = [&](BridgeAlign align) {
        for(Gi::Item* gi: App::grView().selectedItems<Gi::Item>()) {
            const auto curves = gi->curves();
            QRectF bounds;
            for(const Geo::Polyline& curve: curves)
                bounds |= curve.boundingRect();
            const double stepH = bounds.width() / (value + 1);
            const double stepV = bounds.height() / (value + 1);

            auto testAndAdd = [&](const QLineF& test, const Geo::Vertex& fr, const Geo::Vertex& to) {
                const QLineF srcline{fr, to};
                QPointF intersect;
                if(test.intersects(srcline, &intersect) != QLineF::BoundedIntersection) return;
                if(!fr.isArc() && srcline.length() >= footprint)
                    intersect = (fr + to) * 0.5; // середина прямого участка
                addAt(intersect);
            };

            for(int var = 1; var <= int(value); ++var) {
                const QLineF testLineH{
                    {bounds.left() + stepH * var, bounds.bottom() + 1},
                    {bounds.left() + stepH * var, bounds.top() - 1   }
                };
                const QLineF testLineV{
                    {bounds.left() - 1,  bounds.top() + stepV * var},
                    {bounds.right() + 1, bounds.top() + stepV * var}
                };
                for(const Geo::Polyline& curve: curves)
                    for(auto&& [fr, to]: Geo::segments(curve)) {
                        if(align & Horizontally) testAndAdd(testLineH, fr, to);
                        if(align & Vertically) testAndAdd(testLineV, fr, to);
                    }
            }
        }
    };

    switch(auto at = BridgeAlign(ui->cbxBridgeAlignType->currentIndex()); at) {
    case Manually: {
        auto brItem = new Gi::Bridge;
        App::grView().addItem(brItem);
        brItem->setVisible(true);
        brItem->setOpacity(1.0);
        Gi::Bridge::moveBrPtr() = brItem; // ездит за курсором до клика
    } break;
    case Horizontally:
    case Vertically:
    case HorizontallyVertically:
        addHorizontallyVertically(at);
        break;
    case ThroughTheDistance:
        addAlong(false);
        break;
    case EvenlyDround:
        addAlong(true);
        break;
    case Split:
        for(Gi::Item* gi: App::grView().selectedItems<Gi::Item>())
            for(auto&& curve: gi->curves()) {
                if(curve.size() != 2) continue;
                addAt(QLineF{curve.front(), curve.back()}.center());
            }
        break;
    default: break;
    }
}

void Form::updateBridges() {
    Gi::Bridge::lenght() = ui->dsbxBridgeLenght->value();
    Gi::Bridge::toolDiam() = ui->toolHolder->tool().getDiameter(dsbxDepth->value());
    Gi::Bridge::millingSide() = side;
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
    if(Gi::Bridge::moveBrPtr())
        Gi::Bridge::moveBrPtr()->setPos(pos);
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

    // Мостики: восстановить по центрам из УП -- ПОСЛЕ базы, когда сцена
    // (видимость и выделение) уже восстановлена и снапу есть к чему липнуть.
    qDeleteAll(App::grView().items<Gi::Bridge>());
    if(auto it = gcp.params.find(Creator::BridgeLen); it != gcp.params.end())
        ui->dsbxBridgeLenght->setValue(it->second.toDouble());
    if(auto it = gcp.params.find(Creator::BridgeHeight); it != gcp.params.end())
        ui->dsbxBridgeHeight->setValue(it->second.toDouble());
    updateBridges();
    for(const Geo::Polylines& group: gcp.supportCurvess)
        for(const Geo::Polyline& centers: group)
            for(const Geo::Vertex& center: centers) {
                auto* brItem = App::grView().addItem<Gi::Bridge>();
                brItem->setVisible(true);
                brItem->setOpacity(1.0);
                brItem->setPos(center); // NOTE сперва встать: snapedPos ищет collidingItems
                brItem->setPos(brItem->snapedPos(center));
                if(!brItem->ok())
                    delete brItem;
            }
}

} // namespace Profile

#include "moc_profile_form.cpp"
