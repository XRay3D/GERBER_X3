// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "gbr_aperture.h"
#include "gbr_file.h"
#include "settings.h"

#include "doublespinbox.h"
#include "drill/drill_form.h"
#include "thermal/thermal_vars.h"
#include "utils.h"

#include <QtWidgets>
#include <ctre.hpp>

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

std::any Plugin::createPreviewGi(FileInterface* file, GCodePlugin* plugin, std::any param) {

    auto mapPaths = [file](Paths paths) {
        for (auto&& path : paths)
            path = file->transform().map(path);
        return paths;
    };

    auto mapPos = [file](auto pos) {
        pos = file->transform().map(pos);
        return pos;
    };

    if (plugin->type() == ::GCode::Drill) {
        Drill::Preview retData;
        double drillDiameter {};

        auto const gbrFile = static_cast<File*>(file);
        for (auto gbrObj : gbrFile->graphicObjects2()) {
            if (!gbrFile->apertures_.contains(gbrObj.state().aperture()) || gbrObj.state().dCode() != D03)
                continue;

            auto& ap = *gbrFile->apertures_[gbrObj.state().aperture()];

            if (!ap.flashed())
                continue;

            auto name {ap.name()};
            if (ap.withHole()) {
                drillDiameter = ap.drillDiameter();
                name += tr(", drill Ø%1mm").arg(drillDiameter);
            } else if (ap.type() == Circle) {
                drillDiameter = ap.apSize();
            } else {
                drillDiameter = ap.minSize();
            }
            qDebug() << ap.type() << "ap.apSize()" << ap.apSize();
            retData[{gbrObj.state().aperture(), drillDiameter, false, name}].posOrPath.emplace_back(mapPos(gbrObj.state().curPos()));

            // draw aperture
            if (!retData[{gbrObj.state().aperture(), drillDiameter, false, name}].draw.size()) {
                auto state = gbrObj.state();
                state.setCurPos({});
                retData[{gbrObj.state().aperture(), drillDiameter, false, name}].draw = ap.draw(state);
                QTransform transform {};
                transform.rotateRadians(asin(file->transform().m12()));
                for (auto&& path : retData[{gbrObj.state().aperture(), drillDiameter, false, name}].draw)
                    path = transform.map(path);
            }
        }

        return retData;
    }
    if (plugin->type() == ::GCode::Thermal) {
        auto param_ = std::any_cast<Thermal::ThParam2>(param);
        Thermal::PreviewGiMap sourcePreview;
        auto gbrFile = static_cast<File*>(file);

        auto testArea = [&param_](const Paths& paths) {
            const double areaMax = param_.areaMax;
            const double areaMin = param_.areaMin;
            const double area = Area(paths);
            return areaMin <= area && area <= areaMax;
        };

        const ApertureMap& apertures_ = *gbrFile->apertures();

        if (param_.aperture) {
            std::unordered_map<int, Thermal::PreviewGiMapValVec*> thermalNodes;

            for (const auto [dCode, aperture] : apertures_)
                if (aperture->flashed() && !thermalNodes.contains(dCode) && testArea(aperture->draw({})))
                    thermalNodes.emplace(dCode, &sourcePreview[0][aperture->name()]);

            for (const auto& [dCode, aperture] : apertures_)
                if (aperture->flashed() && testArea(aperture->draw({})))
                    for (GraphicObject& go : gbrFile->graphicObjects_)
                        if (thermalNodes.contains(dCode) && go.state().dCode() == D03 && go.state().aperture() == dCode)
                            thermalNodes[dCode]->emplace_back(mapPaths(go.paths()), mapPos(go.state().curPos()));
        }

        if (param_.path) {
            auto& mv = sourcePreview[1][tr("Lines")];

            for (/*const*/ GraphicObject& go : gbrFile->graphicObjects_)
                if (go.state().type() == PrimitiveType::Line
                    && go.state().imgPolarity() == Positive
                    && (go.path().size() == 2 || (go.path().size() == 5 && go.path().front() == go.path().back()))
                    && go.path().front().distTo(go.path().back()) * dScale * 0.3 < apertures_.at(go.state().aperture())->minSize()
                    && testArea(go.paths()))
                    mv.emplace_back(mapPaths(go.paths()), IntPoint {});
        }

        if (param_.pour) {
            auto& mv = sourcePreview[2][tr("Regions")];
            mvector<const GraphicObject*> gos;
            for (const GraphicObject& go : gbrFile->graphicObjects_)
                if (go.state().type() == PrimitiveType::Region
                    && go.state().imgPolarity() == Positive
                    && testArea(go.paths()))
                    gos.push_back(&go);

            std::ranges::sort(gos, {}, [](const GraphicObject* go1) {
                return go1->state().curPos();
            });
            for (auto& go : gos)
                mv.emplace_back(mapPaths(go->paths()), IntPoint {});
        }

        return sourcePreview;
    }
    return {};
}

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
        static constexpr ctll::fixed_string pattern(R"(%FS[LTD]?[AI]X\d{2}Y\d{2}\*)"); // fixed_string("%FS[LTD]?[AI]X\d{2}Y\d{2}\*");
        QTextStream in(&file);
        QString line;
        while (in.readLineInto(&line)) {
            auto data {toU16StrView(line)};
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
            cleanPolygons_ = settings.getValue(chbxCleanPolygons, cleanPolygons_);
            cleanPolygonsDist_ = settings.getValue(dsbxCleanPolygonsDist, cleanPolygonsDist_);
            simplifyRegions_ = settings.getValue(chbxSimplifyRegions, simplifyRegions_);
            skipDuplicates_ = settings.getValue(chbxSkipDuplicates, skipDuplicates_);
            settings.endGroup();
        }
        virtual void writeSettings(MySettings& settings) override {
            settings.beginGroup("Gerber");
            cleanPolygons_ = settings.setValue(chbxCleanPolygons);
            cleanPolygonsDist_ = settings.setValue(dsbxCleanPolygonsDist);
            simplifyRegions_ = settings.setValue(chbxSimplifyRegions);
            skipDuplicates_ = settings.setValue(chbxSkipDuplicates);
            settings.endGroup();
        }
    };
    auto tab = new Tab(parent);
    tab->setWindowTitle("Gerber X3");
    return tab;
}

void Plugin::addToGcForm(FileInterface* file, QComboBox* cbx) {
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
