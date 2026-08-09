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
    : AbstractFilePlugin{parent}
    , Parser(this) {
}

AbstractFile* Plugin::parseFile(const QString& fileName, uint32_t type_) {
    if(type_ != type()) return nullptr;
    QFile file_{fileName};
    if(!file_.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << file_.errorString();
        return nullptr;
    }

    QTextStream in{&file_};
    in.setAutoDetectUnicode(true);
    parseLines(in.readAll(), fileName);
    return file;
}

QIcon drawApertureIcon(AbstractAperture* aperture) {
    QPainterPath painterPath{aperture->draw(State()).toPath()};
    painterPath.addEllipse(QPointF(0, 0), aperture->drillDiameter() * 0.5, aperture->drillDiameter() * 0.5);
    const QRectF rect = painterPath.boundingRect();
    double scale      = static_cast<double>(IconSize) / std::max(rect.width(), rect.height());
    double ky         = -rect.top() * scale;
    double kx         = rect.left() * scale;
    if(rect.width() > rect.height())
        ky += (static_cast<double>(IconSize) - rect.height() * scale) / 2;
    else
        kx -= (static_cast<double>(IconSize) - rect.width() * scale) / 2;
    QPixmap pixmap{IconSize, IconSize};
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
    { // Пользователь мог переключить обработку герберов на плагин file_gerber2.
        MySettings settings;
        settings.beginGroup(u"Gerber2"_s);
        if(settings.value(u"chbxPreferred"_s, false).toBool()) return false;
    }
    QFile file{fileName};
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in{&file};
        QString line;
        while(in.readLineInto(&line)) {
            if(*ctre::search_all<R"(%FS[LTD]?[AI]X\d{2}Y\d{2}\*)">(
                   std::u16string_view{line})
                    .begin())
                return true;
        }
    }
    return false;
}

AbstractFile* Plugin::loadFile(QDataStream& stream) const { return File::load<File>(stream); }

AbstractFile* Plugin::loadFile(std::string_view json) const { return File::load<File>(json); }

QIcon Plugin::icon() const { return decoration(Qt::lightGray, u'G'); }

AbstractFileSettings* Plugin::createSettingsTab(QWidget* parent) {
    class Tab : public AbstractFileSettings, Settings {
        QCheckBox* chbxCleanPolygons;
        QCheckBox* chbxSkipDuplicates;
        QCheckBox* chbxSimplifyRegions;
        DoubleSpinBox* dsbxCleanPolygonsDist;

    public:
        Tab(QWidget* parent = nullptr)
            : AbstractFileSettings{parent} {
            setObjectName(u"tabGerber"_s);

            auto verticalLayout = new QVBoxLayout{this};
            verticalLayout->setObjectName(u"verticalLayout"_s);
            verticalLayout->setContentsMargins(6, 6, 6, 6);

            {
                auto groupBox1 = new QGroupBox{this};
                groupBox1->setObjectName(u"groupBox1"_s);
                groupBox1->setTitle(QApplication::translate("SettingsDialog", "Gerber", nullptr));
                verticalLayout->addWidget(groupBox1);

                chbxCleanPolygons = new QCheckBox{groupBox1};
                chbxCleanPolygons->setObjectName(u"chbxCleanPolygons"_s);

                chbxSimplifyRegions = new QCheckBox{groupBox1};
                chbxSimplifyRegions->setObjectName(u"chbxSimplifyRegions"_s);

                chbxSkipDuplicates = new QCheckBox{groupBox1};
                chbxSkipDuplicates->setObjectName(u"chbxSkipDuplicates"_s);

                dsbxCleanPolygonsDist = new DoubleSpinBox{groupBox1};
                dsbxCleanPolygonsDist->setDecimals(4);
                dsbxCleanPolygonsDist->setObjectName(u"dsbxCleanPolygonsDist"_s);
                dsbxCleanPolygonsDist->setRange(0.0001, 1.0);
                dsbxCleanPolygonsDist->setSingleStep(0.001);

                auto vBoxLayout = new QVBoxLayout{groupBox1};
                vBoxLayout->addWidget(chbxCleanPolygons);
                vBoxLayout->addWidget(chbxSimplifyRegions);
                vBoxLayout->addWidget(chbxSkipDuplicates);
                vBoxLayout->addWidget(dsbxCleanPolygonsDist);
                vBoxLayout->setContentsMargins(6, 9, 6, 6);
            }

            verticalLayout->addItem(new QSpacerItem{20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding});

            chbxCleanPolygons->setText(QApplication::translate("SettingsDialog", "Cleaning Polygons", nullptr));
            chbxSkipDuplicates->setText(QApplication::translate("SettingsDialog", "Skip duplicates", nullptr));
            chbxSimplifyRegions->setText(QApplication::translate("SettingsDialog", "Simplify Regions", nullptr));
        }
        virtual ~Tab() override { }
        virtual void readSettings(MySettings& settings) override {
            settings.beginGroup(u"Gerber"_s);
            cleanPolygons_     = settings.getValue(chbxCleanPolygons, cleanPolygons_);
            cleanPolygonsDist_ = settings.getValue(dsbxCleanPolygonsDist, cleanPolygonsDist_);
            simplifyRegions_   = settings.getValue(chbxSimplifyRegions, simplifyRegions_);
            skipDuplicates_    = settings.getValue(chbxSkipDuplicates, skipDuplicates_);

            settings.endGroup();
        }
        virtual void writeSettings(MySettings& settings) override {
            settings.beginGroup(u"Gerber"_s);
            cleanPolygons_     = settings.setValue(chbxCleanPolygons);
            cleanPolygonsDist_ = settings.setValue(dsbxCleanPolygonsDist);
            simplifyRegions_   = settings.setValue(chbxSimplifyRegions);
            skipDuplicates_    = settings.setValue(chbxSkipDuplicates);

            settings.endGroup();
        }
    };
    auto tab = new Tab{parent};
    tab->setWindowTitle(u"Gerber X3"_s);
    return tab;
}

} // namespace Gerber

#include "moc_gbr_plugin.cpp"
