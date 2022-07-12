// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbr_plugin.h"

#include "drill/gc_drillform.h"
#include "gbr_aperture.h"
#include "gbr_file.h"
#include "gbr_node.h"

#include "doublespinbox.h"
#include "ft_view.h"
#include "settings.h"
#include "thermal/gc_thvars.h"
#include "tool_pch.h"
#include "utils.h"

//#include <QtConcurrent>
#include <QtWidgets>

#include <ctre.hpp> //

namespace Gerber {

const int id1 = qRegisterMetaType<File*>("G::GFile*");

Plugin::Plugin(QObject* parent)
    : FilePlugin(parent)
    , Parser(this) {
}

FileInterface* Plugin::parseFile(const QString& fileName, int type_) {
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

std::any Plugin::createPreviewGi(FileInterface* file, GCodePlugin* plugin) {
    if (plugin->type() == ::GCode::Drill) {
        Drills retData;
        double drillDiameter {};

        auto const gbrFile = static_cast<File*>(file);
        for (auto gbrObj : gbrFile->graphicObjects2()) {
            if (!gbrFile->apertures_.contains(gbrObj.state().aperture()) || gbrObj.state().dCode() != D03)
                continue;

            auto& ap = *gbrFile->apertures_[gbrObj.state().aperture()];

            if (!ap.flashed())
                continue;

            auto name { ap.name() };
            if (ap.withHole()) {
                drillDiameter = ap.drillDiameter();
                name += tr(", drill Ø%1mm").arg(drillDiameter);
            } else if (ap.type() == Circle) {
                drillDiameter = ap.apertureSize();
            }

            retData[{ gbrObj.state().aperture(), drillDiameter, false, name }].posOrPath.emplace_back(file->transform().map(gbrObj.state().curPos()));

            // draw aperture
            if (!retData[{ gbrObj.state().aperture(), drillDiameter, false, name }].draw.size()) {
                auto state = gbrObj.state();
                state.setCurPos({});
                retData[{ gbrObj.state().aperture(), drillDiameter, false, name }].draw = ap.draw(state);
                QTransform transform {};
                transform.rotateRadians(asin(file->transform().m12()));
                for (auto&& path : retData[{ gbrObj.state().aperture(), drillDiameter, false, name }].draw)
                    path = transform.map(path);
            }
        }

        return retData;
    }
    if (plugin->type() == ::GCode::Thermal) {
        Drills retData;
        double drillDiameter {};

        auto const gbrFile = static_cast<File*>(file);
        for (auto gbrObj : gbrFile->graphicObjects2()) {
            if (!gbrFile->apertures_.contains(gbrObj.state().aperture()) || gbrObj.state().dCode() != D03)
                continue;

            auto& ap = *gbrFile->apertures_[gbrObj.state().aperture()];

            if (!ap.flashed())
                continue;

            auto name { ap.name() };
            if (ap.withHole()) {
                drillDiameter = ap.drillDiameter();
                name += tr(", drill Ø%1mm").arg(drillDiameter);
            } else if (ap.type() == Circle) {
                drillDiameter = ap.apertureSize();
            }

            retData[{ gbrObj.state().aperture(), drillDiameter, false, name }].posOrPath.emplace_back(file->transform().map(gbrObj.state().curPos()));

            // draw aperture
            if (!retData[{ gbrObj.state().aperture(), drillDiameter, false, name }].draw.size()) {
                auto state = gbrObj.state();
                state.setCurPos({});
                retData[{ gbrObj.state().aperture(), drillDiameter, false, name }].draw = ap.draw(state);
                QTransform transform {};
                transform.rotateRadians(asin(file->transform().m12()));
                for (auto&& path : retData[{ gbrObj.state().aperture(), drillDiameter, false, name }].draw)
                    path = transform.map(path);
            }
        }

        return retData;
    }
    return {};
}

QIcon drawApertureIcon(AbstractAperture* aperture) {
    QPainterPath painterPath;
    for (const auto& polygon : aperture->draw(State()))
        painterPath.addPolygon(polygon);
    painterPath.addEllipse(QPointF(0, 0), aperture->drillDiameter() * 0.5, aperture->drillDiameter() * 0.5);
    const QRectF rect = painterPath.boundingRect();
    double scale = static_cast<double>(IconSize) / qMax(rect.width(), rect.height());
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
        static constexpr ctll::fixed_string pattern(R"(%FS[LTD]?[AI]X\d{2}Y\d{2}\*)"); // fixed_string("%FS[LTD]?[AI]X\d{2}Y\d{2}\*");
        QTextStream in(&file);
        QString line;
        while (in.readLineInto(&line)) {
            auto data { toU16StrView(line) };
            if (*ctre::range<pattern>(data).begin())
                return true;
        }
    }
    return false;
}

int Plugin::type() const { return int(FileType::Gerber); }

QString Plugin::folderName() const { return tr("Gerber Files"); }

FileInterface* Plugin::createFile() { return new File(); }

QIcon Plugin::icon() const { return decoration(Qt::lightGray, 'G'); }

SettingsTabInterface* Plugin::createSettingsTab(QWidget* parent) {
    class Tab : public SettingsTabInterface, Settings {
        QCheckBox* chbxCleanPolygons;
        QCheckBox* chbxSkipDuplicates;
        QCheckBox* chbxSimplifyRegions;
        DoubleSpinBox* dsbxCleanPolygonsDist;

    public:
        Tab(QWidget* parent = nullptr)
            : SettingsTabInterface(parent) {
            setObjectName(QString::fromUtf8("tabGerber"));
            auto verticalLayout = new QVBoxLayout(this);
            verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
            verticalLayout->setContentsMargins(6, 6, 6, 6);

            auto groupBox = new QGroupBox(this);
            groupBox->setObjectName(QString::fromUtf8("groupBox"));
            auto verticalLayout2 = new QVBoxLayout(groupBox);
            verticalLayout2->setObjectName(QString::fromUtf8("verticalLayout2"));
            verticalLayout2->setContentsMargins(6, 9, 6, 6);

            chbxCleanPolygons = new QCheckBox(groupBox);
            chbxCleanPolygons->setObjectName(QString::fromUtf8("chbxCleanPolygons"));
            verticalLayout2->addWidget(chbxCleanPolygons);

            dsbxCleanPolygonsDist = new DoubleSpinBox(groupBox);
            dsbxCleanPolygonsDist->setObjectName(QString::fromUtf8("dsbxCleanPolygonsDist"));
            dsbxCleanPolygonsDist->setRange(0.0001, 1.0);
            dsbxCleanPolygonsDist->setSingleStep(0.001);
            dsbxCleanPolygonsDist->setDecimals(4);
            verticalLayout2->addWidget(dsbxCleanPolygonsDist);

            chbxSkipDuplicates = new QCheckBox(groupBox);
            chbxSkipDuplicates->setObjectName(QString::fromUtf8("chbxSkipDuplicates"));
            verticalLayout2->addWidget(chbxSkipDuplicates);

            chbxSimplifyRegions = new QCheckBox(groupBox);
            chbxSimplifyRegions->setObjectName(QString::fromUtf8("chbxSimplifyRegions"));
            verticalLayout2->addWidget(chbxSimplifyRegions);

            verticalLayout->addWidget(groupBox);
            auto verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            verticalLayout->addItem(verticalSpacer);

            groupBox->setTitle(QApplication::translate("SettingsDialog", "Gerber", nullptr));
            chbxCleanPolygons->setText(QApplication::translate("SettingsDialog", "Cleaning Polygons", nullptr));
            chbxSkipDuplicates->setText(QApplication::translate("SettingsDialog", "Skip duplicates", nullptr));
            chbxSimplifyRegions->setText(QApplication::translate("SettingsDialog", "Simplify Regions", nullptr));
        }
        virtual ~Tab() override { }
        virtual void readSettings(MySettings& settings) override {
            settings.beginGroup("Gerber");
            m_cleanPolygons = settings.getValue(chbxCleanPolygons, m_cleanPolygons);
            m_cleanPolygonsDist = settings.getValue(dsbxCleanPolygonsDist, m_cleanPolygonsDist);
            m_simplifyRegions = settings.getValue(chbxSimplifyRegions, m_simplifyRegions);
            m_skipDuplicates = settings.getValue(chbxSkipDuplicates, m_skipDuplicates);
            settings.endGroup();
        }
        virtual void writeSettings(MySettings& settings) override {
            settings.beginGroup("Gerber");
            m_cleanPolygons = settings.setValue(chbxCleanPolygons);
            m_cleanPolygonsDist = settings.setValue(dsbxCleanPolygonsDist);
            m_simplifyRegions = settings.setValue(chbxSimplifyRegions);
            m_skipDuplicates = settings.setValue(chbxSkipDuplicates);
            settings.endGroup();
        }
    };
    auto tab = new Tab(parent);
    tab->setWindowTitle("Gerber X3");
    return tab;
}

void Plugin::addToDrillForm(FileInterface* file, QComboBox* cbx) {
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

// class DrillPrGI final : public AbstractDrillPrGI {
// public:
//     explicit DrillPrGI(const GraphicObject* go, int id, Row& row)
//         : AbstractDrillPrGI(row)
//         , apId(id)
//         , gbrObj(go) {
//         auto ap = go->gFile()->apertures()->at(id);
//         sourceDiameter_ = qFuzzyIsNull(ap->drillDiameter()) ? ap->minSize() : ap->drillDiameter();
//         m_sourcePath = drawApetrure(go, id);
//         type_ = GiType::PrApetrure;
//     }

// private:
//     static QPainterPath drawApetrure(const GraphicObject* go, int id) {
//         QPainterPath painterPath;
//         for (QPolygonF polygon : go->paths()) {
//             polygon.append(polygon.first());
//             painterPath.addPolygon(polygon);
//         }
//         const double hole = go->gFile()->apertures()->at(id)->drillDiameter() * 0.5;
//         if (hole != 0.0)
//             painterPath.addEllipse(go->state().curPos(), hole, hole);
//         return painterPath;
//     }
//     const int apId;
//     const GraphicObject* const gbrObj = nullptr;

//    // AbstractDrillPrGI interface
// public:
//    void updateTool() override {
//        if (toolId_ > -1)
//            colorState |= Tool;
//        else
//            colorState &= ~Tool;

//        changeColor();
//    }
//    IntPoint pos() const override { return gbrObj->state().curPos(); }
//    Paths paths() const override { return gbrObj->paths(); }
//    bool fit(double depth) override {
//        return gbrObj->gFile()->apertures()->at(apId)->fit(App::toolHolder().tool(toolId_).getDiameter(depth));
//    }
//};

// FIXME DrillPreviewGiMap Plugin::createDrillPreviewGi(FileInterface* file, mvector<Row>& data) {
//    DrillPreviewGiMap giPeview;
//    auto const gbrFile = reinterpret_cast<File*>(file);
//    const ApertureMap* const m_apertures = gbrFile->apertures();

//    uint count = 0;
//    for (auto [dCode, aperture] : *m_apertures) {
//        (void)dCode;
//        if (ap.flashed())
//            ++count;
//    }

//    std::map<int, std::vector<const GraphicObject*>> cacheApertures;
//    for (auto& go : gbrFile->graphicObjects_)
//        if (go.state().dCode() == D03)
//            cacheApertures[go.state().aperture()].push_back(&go);

//    assert(count == cacheApertures.size()); // assert on != - false

//    data.reserve(count); // !!! reserve для отсутствия реалокаций, так как DrillPrGI хранит ссылки на него !!!
//    for (auto [apDCode, aperture] : *m_apertures) {
//        if (aperture && ap.flashed()) {
//            double drillDiameter = 0;
//            QString name(ap.name());
//            if (ap.withHole()) {
//                drillDiameter = ap.drillDiameter();
//                name += tr(", drill Ø%1mm").arg(drillDiameter);
//            } else if (ap.type() == Circle) {
//                drillDiameter = ap.apertureSize();
//            }

//            data.emplace_back(std::move(name), drawApertureIcon(aperture.get()), apDCode, drillDiameter);
//            for (const GraphicObject* go : cacheApertures[apDCode])
//                giPeview[apDCode].push_back(std::make_shared<DrillPrGI>(go, apDCode, data.back()));
//        }
//    }

//    return giPeview;
//}

// FIXME ThermalPreviewGiMap Plugin::createThermalPreviewGi(FileInterface* file, const ThParam2& param) {
//    ThermalPreviewGiMap sourcePreview;
//    auto gbrFile = static_cast<File*>(file);

//    auto testArea = [&param](const Paths& paths) {
//        const double areaMax = param.areaMax;
//        const double areaMin = param.areaMin;
//        const double area = Area(paths);
//        return areaMin <= area && area <= areaMax;
//    };

//    const ApertureMap& m_apertures = *gbrFile->apertures();
//    /////////////////////////////////////////////////////
//    if (param.aperture) {
//        std::map<int, mvector<std::pair<const Paths*, IntPoint>>*> thermalNodes;

//        for (const auto [dCode, aperture] : m_apertures) {
//            if (ap.flashed() && testArea(ap.draw({}))) {
//                thermalNodes.emplace(dCode, &sourcePreview[0][ap.name()]);
//                //                thermalNodes[dCode] = param.model->appendRow(drawApertureIcon(aperture.get()), ap.name(), param.par);
//            }
//        }
//        for (const auto& [dCode, aperture] : m_apertures) {
//            if (ap.flashed() && testArea(ap.draw({}))) {
//                for (GraphicObject& go : gbrFile->graphicObjects_) {
//                    if (thermalNodes.contains(dCode)
//                        && go.state().dCode() == D03
//                        && go.state().aperture() == dCode) {
//                        thermalNodes[dCode]->emplace_back(&go.paths(), go.state().curPos());
//                        //                            workers.emplace_back(&go, thermalNodes[dCode], "", ctr++);
//                    }
//                }
//            }
//        }
//    }
//    /////////////////////////////////////////////////////
//    if (param.path) {
//        auto& mv = sourcePreview[1][tr("Lines")];

//        //            thermalNodes[Line] = param.model->appendRow(QIcon(), tr("Lines"), param.par);
//        for (/*const*/ GraphicObject& go : gbrFile->graphicObjects_) {
//            if (go.state().type() == PrimitiveType::Line
//                && go.state().imgPolarity() == Positive
//                && (go.path().size() == 2 || (go.path().size() == 5 && go.path().front() == go.path().back()))
//                && go.path().front().distTo(go.path().back()) * dScale * 0.3 < m_apertures.at(go.state().aperture())->minSize()
//                && testArea(go.paths())) {
//                mv.emplace_back(&go.paths(), IntPoint {});
//                //                workers.emplace_back(&go, thermalNodes[Line], tr("Line"), ctr++);
//            }
//        }
//    }
//    /////////////////////////////////////////////////////
//    if (param.pour) {
//        auto& mv = sourcePreview[2][tr("Regions")];
//        //        thermalNodes[Region] = param.model->appendRow(QIcon(), tr("Regions"), param.par);
//        mvector<const GraphicObject*> gos;
//        for (const GraphicObject& go : gbrFile->graphicObjects_) {
//            if (go.state().type() == PrimitiveType::Region
//                && go.state().imgPolarity() == Positive
//                && testArea(go.paths())) {
//                gos.push_back(&go);
//            }
//        }
//        std::ranges::sort(gos, {}, [](const GraphicObject* go1) {
//            //            return go1->paths() < go2->paths();
//            return go1->state().curPos();
//        });
//        for (auto& go : gos) {
//            mv.emplace_back(&go->paths(), IntPoint {});
//            //            workers.emplace_back(go, thermalNodes[Region], tr("Region"), ctr++);
//        }
//    }

//    //    std::vector<std::future<void>> futures;
//    //    for (size_t i = 0, c = QThread::idealThreadCount(); i < workers.size(); i += c) {
//    //        futures.clear();
//    //        for (auto&& wr : workers.mid(i, c))
//    //            futures.emplace_back(std::async(std::launch::async, creator, wr));
//    //        for (auto&& future : futures)
//    //            future.wait();
//    //    }

//    return sourcePreview;
//}

} // namespace Gerber
