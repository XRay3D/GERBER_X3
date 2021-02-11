// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gbrplugin.h"

#include "gbraperture.h"
#include "gbrfile.h"
#include "gbrnode.h"

#include "doublespinbox.h"
#include "drillpreviewgi.h"
#include "ft_view.h"
#include "settings.h"
#include "tool.h"

#include <thermalmodel.h>
#include <thermalnode.h>
#include <thermalpreviewitem.h>

#include <QtConcurrent>
#include <QtWidgets>

#include <ctre.hpp> //

#include "leakdetector.h"

namespace Gerber {

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

const int id1 = qRegisterMetaType<File*>("G::GFile*");

Plugin::Plugin(QObject* parent)
    : QObject(parent)
    , Parser(this)
{
}

FileInterface* Plugin::parseFile(const QString& fileName, int type_)
{
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

QIcon Plugin::drawApertureIcon(AbstractAperture* aperture) const
{
    QPainterPath painterPath;
    for (QPolygonF polygon : aperture->draw(State()))
        painterPath.addPolygon(polygon);
    painterPath.addEllipse(QPointF(0, 0), aperture->drillDiameter() * 0.5, aperture->drillDiameter() * 0.5);
    const QRectF rect = painterPath.boundingRect();
    qreal scale = static_cast<double>(IconSize) / qMax(rect.width(), rect.height());
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

QIcon Plugin::drawRegionIcon(const GraphicObject& go) const
{
    static QMutex m;
    QMutexLocker l(&m);

    QPainterPath painterPath;

    for (QPolygonF polygon : go.paths())
        painterPath.addPolygon(polygon);

    const QRectF rect = painterPath.boundingRect();

    qreal scale = static_cast<double>(IconSize) / qMax(rect.width(), rect.height());

    double ky = rect.bottom() * scale;
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
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    painter.drawPath(painterPath);
    return QIcon(pixmap);
}

bool Plugin::thisIsIt(const QString& fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        QString line;
        while (in.readLineInto(&line)) {
            using namespace ctre::literals;
            if (auto m = ctre::match<"^%(FS[LTD]?[AI]X\\d{2}Y\\d{2})\\*%$">(line.toStdString()))
                return true;
            static const QRegularExpression match(QStringLiteral("%FS[LTD]?[AI]X\\d{2}Y\\d{2}\\*%"));
            if (line.startsWith('%') && match.match(line).hasMatch())
                return true;
        }
    }
    return false;
}

QObject* Plugin::getObject() { return this; }

int Plugin::type() const { return int(FileType::Gerber); }

QString Plugin::folderName() const { return tr("Gerber Files"); }

FileInterface* Plugin::createFile() { return new File(); }

QJsonObject Plugin::info() const
{
    return QJsonObject {
        { "Name", "Gerber X3" },
        { "Version", "1.0" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "Opening GerberX3 files, with support for all kinds of aperture macros and components." },
    };
}

std::pair<SettingsTabInterface*, QString> Plugin::createSettingsTab(QWidget* parent)
{
    class Tab : public SettingsTabInterface, Settings {
        QCheckBox* chbxCleanPolygons;
        QCheckBox* chbxSkipDuplicates;
        QCheckBox* chbxSimplifyRegions;
        DoubleSpinBox* dsbxCleanPolygonsDist;

    public:
        Tab(QWidget* parent = nullptr)
            : SettingsTabInterface(parent)
        {
            setObjectName(QString::fromUtf8("tabDxf"));
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
        virtual void readSettings(MySettings& settings) override
        {
            settings.beginGroup("Gerber");
            m_cleanPolygons = settings.getValue(chbxCleanPolygons, m_cleanPolygons);
            m_cleanPolygonsDist = settings.getValue(dsbxCleanPolygonsDist, m_cleanPolygonsDist);
            m_simplifyRegions = settings.getValue(chbxSimplifyRegions, m_simplifyRegions);
            m_skipDuplicates = settings.getValue(chbxSkipDuplicates, m_skipDuplicates);
            settings.endGroup();
        }
        virtual void writeSettings(MySettings& settings) override
        {
            settings.beginGroup("Gerber");
            m_cleanPolygons = settings.setValue(chbxCleanPolygons);
            m_cleanPolygonsDist = settings.setValue(dsbxCleanPolygonsDist);
            m_simplifyRegions = settings.setValue(chbxSimplifyRegions);
            m_skipDuplicates = settings.setValue(chbxSkipDuplicates);
            settings.endGroup();
        }
    };
    return { new Tab(parent), "Gerber X3" };
}

void Plugin::addToDrillForm(FileInterface* file, QComboBox* cbx)
{
    if (static_cast<File*>(file)->flashedApertures()) {
        cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
        QPixmap pixmap(IconSize, IconSize);
        QColor color(file->color());
        color.setAlpha(255);
        pixmap.fill(color);
        cbx->setItemData(cbx->count() - 1, QIcon(pixmap), Qt::DecorationRole);
        cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
    }
}

class DrillPrGI final : public AbstractDrillPrGI {
public:
    explicit DrillPrGI(const GraphicObject* go, int id, Row& row)
        : AbstractDrillPrGI(row)
        , apId(id)
        , gbrObj(go)
    {
        auto ap = go->gFile()->apertures()->at(id);
        m_sourceDiameter = qFuzzyIsNull(ap->drillDiameter()) ? ap->minSize() : ap->drillDiameter();
        m_sourcePath = drawApetrure(go, id);
        m_type = GiType::PrApetrure;
    }

private:
    static QPainterPath drawApetrure(const GraphicObject* go, int id)
    {
        QPainterPath painterPath;
        for (QPolygonF polygon : go->paths()) {
            polygon.append(polygon.first());
            painterPath.addPolygon(polygon);
        }
        const double hole = go->gFile()->apertures()->at(id)->drillDiameter() * 0.5;
        if (hole != 0.0)
            painterPath.addEllipse(go->state().curPos(), hole, hole);
        return painterPath;
    }
    const int apId;
    const GraphicObject* const gbrObj = nullptr;

    // AbstractDrillPrGI interface
public:
    void updateTool() override
    {
        if (row.toolId > -1)
            colorState |= Tool;
        else
            colorState &= ~Tool;

        changeColor();
    }
    IntPoint pos() const override { return gbrObj->state().curPos(); }
    Paths paths() const override { return gbrObj->paths(); }
    bool fit(double depth) override
    {
        return gbrObj->gFile()->apertures()->at(apId)->fit(App::toolHolder().tool(row.toolId).getDiameter(depth));
    }
};

DrillPreviewGiMap Plugin::createDrillPreviewGi(FileInterface* file, mvector<Row>& data)
{
    DrillPreviewGiMap giPeview;
    auto const gbrFile = reinterpret_cast<File*>(file);
    const ApertureMap* const m_apertures = gbrFile->apertures();

    uint count = 0;
    for (auto [dCode, aperture] : *m_apertures) {
        (void)dCode;
        if (aperture->flashed())
            ++count;
    }

    std::map<int, std::vector<const GraphicObject*>> cacheApertures;
    for (auto& go : gbrFile->m_graphicObjects)
        if (go.state().dCode() == D03)
            cacheApertures[go.state().aperture()].push_back(&go);

    assert(count == cacheApertures.size()); // assert on != - false

    data.reserve(count); // !!! reserve для отсутствия реалокаций, так как DrillPrGI хранит ссылки на него !!!
    for (auto [apDCode, aperture] : *m_apertures) {
        if (aperture && aperture->flashed()) {
            double drillDiameter = 0;
            QString name(aperture->name());
            if (aperture->withHole()) {
                drillDiameter = aperture->drillDiameter();
                name += tr(", drill Ø%1mm").arg(drillDiameter);
            } else if (aperture->type() == Circle) {
                drillDiameter = aperture->apertureSize();
            }

            data.emplace_back(std::move(name), drawApertureIcon(aperture.get()), apDCode, drillDiameter);
            for (const GraphicObject* go : cacheApertures[apDCode])
                giPeview[apDCode].push_back(std::make_shared<DrillPrGI>(go, apDCode, data.back()));
        }
    }

    return giPeview;
}

class ThermalPreviewItem final : public AbstractThermPrGi {
    const GraphicObject& grob;

public:
    ThermalPreviewItem(const GraphicObject& go, Tool& tool)
        : AbstractThermPrGi(tool)
        , grob(go)
    {
        for (QPolygonF polygon : grob.paths()) {
            polygon.append(polygon.first());
            sourcePath.addPolygon(polygon);
        }
    }
    IntPoint pos() const override { return grob.state().curPos(); }
    Paths paths() const override { return grob.paths(); }
    void redraw() override
    {
        if (double d = tool.getDiameter(tool.depth()); cashedPath.empty() || !qFuzzyCompare(diameter, d)) {
            diameter = d;
            ClipperOffset offset;
            offset.AddPaths(grob.paths(), jtRound, etClosedPolygon);
            offset.Execute(cashedPath, diameter * uScale * 0.5); // toolpath
            offset.Clear();
            offset.AddPaths(cashedPath, jtMiter, etClosedLine);
            offset.Execute(cashedFrame, diameter * uScale * 0.1); // frame
            for (Path& path : cashedPath)
                path.push_back(path.front());
        }
        if (qFuzzyIsNull(m_node->tickness()) && m_node->count()) {
            m_bridge.clear();
        } else {
            Clipper clipper;
            clipper.AddPaths(cashedFrame, ptSubject, true);
            const auto rect(sourcePath.boundingRect());
            const IntPoint& center(rect.center());
            const double radius = sqrt((rect.width() + diameter) * (rect.height() + diameter)) * uScale;
            const auto fp(sourcePath.toFillPolygons());
            for (int i = 0; i < m_node->count(); ++i) { // Gaps
                ClipperOffset offset;
                double angle = i * 2 * M_PI / m_node->count() + qDegreesToRadians(m_node->angle());
                offset.AddPath({ center,
                                   IntPoint(
                                       static_cast<cInt>((cos(angle) * radius) + center.X),
                                       static_cast<cInt>((sin(angle) * radius) + center.Y)) },
                    jtSquare, etOpenButt);
                Paths paths;
                offset.Execute(paths, (m_node->tickness() + diameter) * uScale * 0.5);
                clipper.AddPath(paths.front(), ptClip, true);
            }
            clipper.Execute(ctIntersection, m_bridge, pftPositive);
        }
        { // cut
            Clipper clipper;
            clipper.AddPaths(cashedPath, ptSubject, false);
            clipper.AddPaths(m_bridge, ptClip, true);
            clipper.Execute(ctDifference, previewPaths, pftPositive);
        }
        painterPath = QPainterPath();
        for (QPolygonF polygon : previewPaths) {
            painterPath.moveTo(polygon.first());
            for (QPointF& pt : polygon)
                painterPath.lineTo(pt);
        }
        if (isEmpty == -1)
            isEmpty = previewPaths.empty();
        if (static_cast<bool>(isEmpty) != previewPaths.empty()) {
            isEmpty = previewPaths.empty();
            changeColor();
        }
        update();
    }
};

ThermalPreviewGiVec Plugin::createThermalPreviewGi(FileInterface* file, const ThParam2& param, Tool& tool)
{
    ThermalPreviewGiVec m_sourcePreview;
    auto gbrFile = static_cast<File*>(file);

    const ApertureMap& m_apertures = *gbrFile->apertures();

    struct Worker {
        const GraphicObject* go;
        ThermalNode* node;
        QString name;
        int strageIdx = -1;
    };

    param.model->appendRow(QIcon(), tr("All"), param.par);

    mvector<Worker> map;
    auto creator = [this, &m_sourcePreview, &tool, &param](Worker w) {
        static QMutex m;
        auto& [go, node, name, strageIdx] = w;
        auto item = std::make_shared<ThermalPreviewItem>(*go, tool);
        //connect(item, &ThermalPreviewItem::selectionChanged, this, &ThermalForm::setSelection);
        item->setToolTip(name);
        QMutexLocker lock(&m);
        m_sourcePreview.push_back(item);
        node->append(new ThermalNode(drawRegionIcon(*go), name, param.par, go->state().curPos(), item.get(), param.model));
    };

    enum {
        Region = -2,
        Line
    };

    int ctr = 0;

    auto testArea = [&param](const Paths& paths) {
        const double areaMax = param.areaMax;
        const double areaMin = param.areaMin;
        const double area = Area(paths);
        return areaMin <= area && area <= areaMax;
    };

#if _MSVC_LANG >= 201705L
    using ThermalNodes = std::map<int, ThermalNode*>;
#else
    struct ThermalNodes : std::map<int, ThermalNode*> {
        bool contains(int key) const { return find(key) != end(); }
    };
#endif

    ThermalNodes thermalNodes;
    if (param.perture) {
        for (auto [dCode, aperture] : m_apertures) {
            if (aperture->flashed() && testArea(aperture->draw({}))) {
                thermalNodes[dCode] = param.model->appendRow(drawApertureIcon(aperture.get()), aperture->name(), param.par);
            }
        }
        for (auto [dCode, aperture] : m_apertures) {
            if (aperture->flashed()) {
                for (const GraphicObject& go : gbrFile->m_graphicObjects) {
                    if (thermalNodes.contains(dCode)
                        && go.state().dCode() == D03
                        && go.state().aperture() == dCode) {
                        map.push_back({ &go, thermalNodes[dCode], "", ctr++ });
                    }
                }
            }
        }
    }

    if (param.path) {
        thermalNodes[Line] = param.model->appendRow(QIcon(), tr("Lines"), param.par);
        for (const GraphicObject& go : gbrFile->m_graphicObjects) {
            if (go.state().type() == PrimitiveType::Line
                && go.state().imgPolarity() == Positive
                && (go.path().size() == 2 || (go.path().size() == 5 && go.path().front() == go.path().back()))
                && go.path().front().distTo(go.path().back()) * dScale * 0.3 < m_apertures.at(go.state().aperture())->minSize()
                && testArea(go.paths())) {
                map.push_back({ &go, thermalNodes[Line], tr("Line"), ctr++ });
            }
        }
    }

    if (param.pour) {
        thermalNodes[Region] = param.model->appendRow(QIcon(), tr("Regions"), param.par);
        mvector<const GraphicObject*> gos;
        for (const GraphicObject& go : gbrFile->m_graphicObjects) {
            if (go.state().type() == PrimitiveType::Region
                && go.state().imgPolarity() == Positive
                && testArea(go.paths())) {
                gos.push_back(&go);
            }
        }
        std::sort(gos.begin(), gos.end(), [](const GraphicObject* go1, const GraphicObject* go2) {
            //            return go1->paths() < go2->paths();
            return go1->state().curPos() < go2->state().curPos();
        });
        for (auto& go : gos) {
            map.push_back({ go, thermalNodes[Region], tr("Region"), ctr++ });
        }
    }

#ifdef QT_DEBUG
    for (auto& worker : map) {
        creator(worker);
    }
#else
    for (size_t i = 0, c = QThread::idealThreadCount(); i < map.size(); i += c) {
        auto m(map.mid(i, c));
        QFuture<void> future = QtConcurrent::map(m, creator);
        future.waitForFinished();
    }
#endif

    return m_sourcePreview;
}

} // namespace Gerber
