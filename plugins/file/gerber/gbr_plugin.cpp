// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbr_plugin.h"
#include "gbr_aperture.h"
#include "gbr_file.h"
#include "settings.h"

#include "doublespinbox.h"
#include "utils.h"

// #include "drill/drill_form.h"
// #include "thermal/thermal_vars.h"

#include <QtWidgets>
#include <ctre.hpp>

namespace Gerber {

const int id1 = qRegisterMetaType<File*>("G::GFile*");

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin(parent)
    , Parser(this) {
}

AbstractFile* Plugin::parseFile(const QString& fileName, int type_) {
    if (type_ != type())
        return nullptr;
    QFile file_(fileName);
    if (!file_.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    QTextStream in(&file_);
    in.setAutoDetectUnicode(true);
    parseLines(in.readAll(), fileName);
    return file;
}

// std::any Plugin::getDataForGC(AbstractFile* file, std::any param) {
//      QTransform t {file->transform()};
//      auto mapPaths = [t](Paths paths) {
//          for (auto&& path : paths)
//              path = t.map(path);
//          return paths;
//      };

//    auto mapPos = [t](auto pos) {
//        pos = t.map(pos);
//        return pos;
//    };

//    // TODO возвращает лишь пути с именем по критериям и центральными точками. non intrusive

//    // FIXME   if (plugin->type() == ::GCode::Drill) {
//    //        Drilling::Preview retData;
//    //        double drillDiameter {};

//    //        auto const gbrFile = static_cast<File*>(file);
//    //        for (auto gbrObj : gbrFile->graphicObjects2()) {
//    //            if (!gbrFile->apertures_.contains(gbrObj.state().aperture()) || gbrObj.state().dCode() != D03)
//    //                continue;

//    //            auto& ap = *gbrFile->apertures_[gbrObj.state().aperture()];

//    //            if (!ap.flashed())
//    //                continue;

//    //            auto name {ap.name()};
//    //            if (ap.withHole()) {
//    //                drillDiameter = ap.drillDiameter();
//    //                name += tr(", drill Ø%1mm").arg(drillDiameter);
//    //            } else if (ap.type() == Circle) {
//    //                drillDiameter = ap.apSize();
//    //            } else {
//    //                drillDiameter = ap.minSize();
//    //            }
//    //            qDebug() << ap.type() << "ap.apSize()" << ap.apSize();
//    //            retData[{gbrObj.state().aperture(), drillDiameter, false, name}].posOrPath.emplace_back(mapPos(gbrObj.state().curPos()));

//    //            // draw aperture
//    //            if (!retData[{gbrObj.state().aperture(), drillDiameter, false, name}].draw.size()) {
//    //                auto state = gbrObj.state();
//    //                state.setCurPos({});
//    //                retData[{gbrObj.state().aperture(), drillDiameter, false, name}].draw = ap.draw(state);
//    //                QTransform transform {};
//    //                transform.rotate(file->transform().angle);
//    //                for (auto&& path : retData[{gbrObj.state().aperture(), drillDiameter, false, name}].draw)
//    //                    path = transform.map(path);
//    //            }
//    //        }

//    // FIXME       return retData;
//    //    }
//    //    if (plugin->type() == ::GCode::Thermal) {
//    //        auto param_ = std::any_cast<Thermal::ThParam2>(param);
//    //        Thermal::PreviewGiMap sourcePreview;
//    //        auto gbrFile = static_cast<File*>(file);

//    //        auto testArea = [&param_](const Paths& paths) {
//    //            const double areaMax = param_.areaMax;
//    //            const double areaMin = param_.areaMin;
//    //            const double area = Area(paths);
//    //            return areaMin <= area && area <= areaMax;
//    //        };

//    //        const ApertureMap& apertures_ = *gbrFile->apertures();

//    //        if (param_.aperture) {
//    //            std::unordered_map<int, Thermal::PreviewGiMapValVec*> thermalNodes;

//    //            for (const auto [dCode, aperture] : apertures_)
//    //                if (aperture->flashed() && !thermalNodes.contains(dCode) && testArea(aperture->draw({})))
//    //                    thermalNodes.emplace(dCode, &sourcePreview[0][aperture->name()]);

//    //            for (const auto& [dCode, aperture] : apertures_)
//    //                if (aperture->flashed() && testArea(aperture->draw({})))
//    //                    for (GraphicObject& go : gbrFile->graphicObjects_)
//    //                        if (thermalNodes.contains(dCode) && go.state().dCode() == D03 && go.state().aperture() == dCode)
//    //                            thermalNodes[dCode]->emplace_back(mapPaths(go.paths()), mapPos(go.state().curPos()));
//    //        }

//    //        if (param_.path) {
//    //            auto& mv = sourcePreview[1][tr("Lines")];

//    //            for (/*const*/ GraphicObject& go : gbrFile->graphicObjects_)
//    //                if (go.state().type() == PrimitiveType::Line
//    //                    && go.state().imgPolarity() == Positive
//    //                    && (go.path().size() == 2 || (go.path().size() == 5 && go.path().front() == go.path().back()))
//    //                    && go.path().front().distTo(go.path().back()) * dScale * 0.3 < apertures_.at(go.state().aperture())->minSize()
//    //                    && testArea(go.paths()))
//    //                    mv.emplace_back(mapPaths(go.paths()), Point {});
//    //        }

//    //        if (param_.pour) {
//    //            auto& mv = sourcePreview[2][tr("Regions")];
//    //            mvector<const GraphicObject*> gos;
//    //            for (const GraphicObject& go : gbrFile->graphicObjects_)
//    //                if (go.state().type() == PrimitiveType::Region
//    //                    && go.state().imgPolarity() == Positive
//    //                    && testArea(go.paths()))
//    //                    gos.push_back(&go);

//    //            std::ranges::sort(gos, {}, [](const GraphicObject* go1) {
//    //                return go1->state().curPos();
//    //            });
//    //            for (auto& go : gos)
//    //                mv.emplace_back(mapPaths(go->paths()), Point {});
//    //        }

//    //        return sourcePreview;
//    //    }
//    return {};
//}

QIcon drawApertureIcon(AbstractAperture* aperture) {
    QPainterPath painterPath;
    for (const auto& polygon : aperture->draw(State()))
        painterPath.addPolygon(polygon);
    painterPath.addEllipse(QPointF(0, 0), aperture->drillDiameter() * 0.5, aperture->drillDiameter() * 0.5);
    const QRectF rect = painterPath.boundingRect();
    double scale = static_cast<double>(IconSize) / std::max(rect.width(), rect.height());
    double ky = -rect.top() * scale;
    double kx = rect.left() * scale;
    if (rect.width() > rect.height())
        ky += (static_cast<double>(IconSize) - rect.height() * scale) / 2;
    else
        kx -= (static_cast<double>(IconSize) - rect.width() * scale) / 2;
    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.translate(-kx, ky);
    painter.scale(scale, scale);
    painter.drawPath(painterPath);
    return QIcon(pixmap);
}

bool Plugin::thisIsIt(const QString& fileName) {
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        QString line;
        while (in.readLineInto(&line)) {
            auto data {toU16StrView(line)};
            if (*ctre::range<R"(%FS[LTD]?[AI]X\d{2}Y\d{2}\*)">(data).begin())
                return true;
        }
    }
    return false;
}

AbstractFile* Plugin::loadFile(QDataStream& stream) const { return File::load<File>(stream); }

QIcon Plugin::icon() const { return decoration(Qt::lightGray, 'G'); }

AbstractFileSettings* Plugin::createSettingsTab(QWidget* parent) {
    class Tab : public AbstractFileSettings, Settings {
        QCheckBox* chbxCleanPolygons;
        QCheckBox* chbxSkipDuplicates;
        QCheckBox* chbxSimplifyRegions;
        DoubleSpinBox* dsbxCleanPolygonsDist;

        QRadioButton* rbClipperOffset;
        QRadioButton* rbMinkowskiSum;

    public:
        Tab(QWidget* parent = nullptr)
            : AbstractFileSettings(parent) {
            setObjectName(QString::fromUtf8("tabGerber"));

            auto verticalLayout = new QVBoxLayout(this);
            verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
            verticalLayout->setContentsMargins(6, 6, 6, 6);

            {
                auto groupBox1 = new QGroupBox(this);
                groupBox1->setObjectName(QString::fromUtf8("groupBox1"));
                groupBox1->setTitle(QApplication::translate("SettingsDialog", "Gerber", nullptr));
                verticalLayout->addWidget(groupBox1);

                chbxCleanPolygons = new QCheckBox(groupBox1);
                chbxCleanPolygons->setObjectName(QString::fromUtf8("chbxCleanPolygons"));

                chbxSimplifyRegions = new QCheckBox(groupBox1);
                chbxSimplifyRegions->setObjectName(QString::fromUtf8("chbxSimplifyRegions"));

                chbxSkipDuplicates = new QCheckBox(groupBox1);
                chbxSkipDuplicates->setObjectName(QString::fromUtf8("chbxSkipDuplicates"));

                dsbxCleanPolygonsDist = new DoubleSpinBox(groupBox1);
                dsbxCleanPolygonsDist->setDecimals(4);
                dsbxCleanPolygonsDist->setObjectName(QString::fromUtf8("dsbxCleanPolygonsDist"));
                dsbxCleanPolygonsDist->setRange(0.0001, 1.0);
                dsbxCleanPolygonsDist->setSingleStep(0.001);

                auto vBoxLayout = new QVBoxLayout(groupBox1);
                vBoxLayout->addWidget(chbxCleanPolygons);
                vBoxLayout->addWidget(chbxSimplifyRegions);
                vBoxLayout->addWidget(chbxSkipDuplicates);
                vBoxLayout->addWidget(dsbxCleanPolygonsDist);
                vBoxLayout->setContentsMargins(6, 9, 6, 6);
            }

            {
                auto groupBox2 = new QGroupBox(this);
                groupBox2->setObjectName(QString::fromUtf8("groupBox2"));
                groupBox2->setTitle(QApplication::translate("SettingsDialog", "Wire Creation Method", nullptr));
                verticalLayout->addWidget(groupBox2);

                rbClipperOffset = new QRadioButton(groupBox2);
                rbClipperOffset->setObjectName(QString::fromUtf8("rbClipperOffset"));

                rbMinkowskiSum = new QRadioButton(groupBox2);
                rbMinkowskiSum->setObjectName(QString::fromUtf8("rbMinkowskiSum"));
                auto vBoxLayout = new QVBoxLayout(groupBox2);
                vBoxLayout->setContentsMargins(6, 9, 6, 6);
                vBoxLayout->addWidget(rbClipperOffset);
                vBoxLayout->addWidget(rbMinkowskiSum);
            }
            verticalLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

            chbxCleanPolygons->setText(QApplication::translate("SettingsDialog", "Cleaning Polygons", nullptr));
            chbxSkipDuplicates->setText(QApplication::translate("SettingsDialog", "Skip duplicates", nullptr));
            chbxSimplifyRegions->setText(QApplication::translate("SettingsDialog", "Simplify Regions", nullptr));

            rbClipperOffset->setText(QApplication::translate("SettingsDialog", "Clipper Offset", nullptr));
            rbClipperOffset->setToolTip(QApplication::translate("SettingsDialog", "Faster", nullptr));
            rbMinkowskiSum->setText(QApplication::translate("SettingsDialog", "Minkowski Sum", nullptr));
            rbMinkowskiSum->setToolTip(QApplication::translate("SettingsDialog", "Better, can cause glitches", nullptr));
        }
        virtual ~Tab() override { }
        virtual void readSettings(MySettings& settings) override {
            settings.beginGroup("Gerber");
            cleanPolygons_ = settings.getValue(chbxCleanPolygons, cleanPolygons_);
            cleanPolygonsDist_ = settings.getValue(dsbxCleanPolygonsDist, cleanPolygonsDist_);
            simplifyRegions_ = settings.getValue(chbxSimplifyRegions, simplifyRegions_);
            skipDuplicates_ = settings.getValue(chbxSkipDuplicates, skipDuplicates_);

            wireMinkowskiSum_ = settings.getValue(rbMinkowskiSum, wireMinkowskiSum_);
            rbClipperOffset->setChecked(!wireMinkowskiSum_);
            settings.endGroup();
        }
        virtual void writeSettings(MySettings& settings) override {
            settings.beginGroup("Gerber");
            cleanPolygons_ = settings.setValue(chbxCleanPolygons);
            cleanPolygonsDist_ = settings.setValue(dsbxCleanPolygonsDist);
            simplifyRegions_ = settings.setValue(chbxSimplifyRegions);
            skipDuplicates_ = settings.setValue(chbxSkipDuplicates);

            wireMinkowskiSum_ = settings.setValue(rbMinkowskiSum);
            settings.endGroup();
        }
    };
    auto tab = new Tab(parent);
    tab->setWindowTitle("Gerber X3");
    return tab;
}

void Plugin::addToGcForm(AbstractFile* file, QComboBox* cbx) {
    if (static_cast<File*>(file)->flashedApertures() && cbx) {
        cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
        QPixmap pixmap(IconSize, IconSize);
        QColor color(file->color());
        color.setAlpha(255);
        pixmap.fill(color);
        cbx->setItemData(cbx->count() - 1, QIcon(pixmap), Qt::DecorationRole);
        cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
    }
}

} // namespace Gerber

#include "moc_gbr_plugin.cpp"
