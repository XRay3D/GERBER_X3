// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gbrplugin.h"

#include "gbrnode.h"

#include "doublespinbox.h"
#include "drillpreviewgi.h"
#include "settings.h"
#include "tool.h"
#include "treeview.h"

#include <QtWidgets>

#include "leakdetector.h"

namespace Gerber {

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

const int id1 = qRegisterMetaType<Gerber::File*>("G::GFile*");

Plugin::Plugin(QObject* parent)
    : QObject(parent)
    , Parser(this)
{
}

FileInterface* Plugin::parseFile(const QString& fileName, int type_)
{
    if (type_ != type())
        return nullptr;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    QTextStream in(&file);
    parseLines(in.readAll(), fileName);
    return m_file = Parser::file;
}

QIcon Plugin::drawApertureIcon(AbstractAperture* aperture)
{
    QPainterPath painterPath;
    for (QPolygonF polygon : aperture->draw(Gerber::State()))
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

bool Plugin::thisIsIt(const QString& fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        static const QRegExp match(QStringLiteral("%FS[LTD]?[AI]X\\d{2}Y\\d{2}\\*%"));
        QString line;
        while (in.readLineInto(&line)) {
            if (line.startsWith('%') && match.exactMatch(line))
                return true;
        }
    }
    return false;
}

QObject* Plugin::getObject() { return this; }

int Plugin::type() const { return int(FileType::Gerber); }

NodeInterface* Plugin::createNode(FileInterface* file) { return new Node(file->id()); }

std::shared_ptr<FileInterface> Plugin::createFile() { return std::make_shared<File>(); }

void Plugin::setupInterface(App* a) { app.set(a); }

void Plugin::createMainMenu(QMenu& menu, FileTreeView* tv)
{
    menu.addAction(QIcon::fromTheme("document-close"), tr("&Close All Files"), [tv] {
        if (QMessageBox::question(tv, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            tv->closeFiles();
    });
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
        virtual ~Tab() override { qDebug(__FUNCTION__); }

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
    explicit DrillPrGI(const Gerber::GraphicObject* go, int id, Row& row)
        : AbstractDrillPrGI(id, row)
        , gbrObj(go)
    {
        auto ap = go->gFile()->apertures()->at(id);
        m_sourceDiameter = qFuzzyIsNull(ap->drillDiameter()) ? ap->minSize() : ap->drillDiameter();
        m_sourcePath = drawApetrure(go, id);
        m_type = GiType::PrApetrure;
    }

private:
    static QPainterPath drawApetrure(const Gerber::GraphicObject* go, int id)
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
    //    static QPainterPath drawPoly(const Gerber::GraphicObject& go)
    //    {
    //    }

    const Gerber::GraphicObject* const gbrObj = nullptr;

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
    Point64 pos() const override { return gbrObj->state().curPos(); }
    Paths paths() const override { return gbrObj->paths(); }
    bool fit(double depth) override
    {
        return gbrObj->gFile()->apertures()->at(id)->fit(App::toolHolder().tool(row.toolId).getDiameter(depth));
    }
};

DrillPreviewGiMap Plugin::createtDrillPreviewGi(FileInterface* file, std::vector<Row>& data)
{
    DrillPreviewGiMap giPeview;
    auto const gbrFile = reinterpret_cast<File*>(file);
    const ApertureMap* const m_apertures = gbrFile->apertures();

    uint count = 0;
    for (auto [dCode, aperture] : *m_apertures) {
        (void)dCode;
        if (aperture->isFlashed())
            ++count;
    }

    std::map<int, std::vector<const GraphicObject*>> cacheApertures;
    for (auto& go : *gbrFile)
        if (go.state().dCode() == Gerber::D03)
            cacheApertures[go.state().aperture()].push_back(&go);

    assert(count == cacheApertures.size()); // assert on != - false
    data.reserve(count);
    for (auto [dCode, aperture] : *m_apertures) {
        if (aperture && aperture->isFlashed()) {
            double drillDiameter = 0;
            QString name(aperture->name());
            if (aperture->withHole()) {
                drillDiameter = aperture->drillDiameter();
                name += tr(", drill Ø%1mm").arg(drillDiameter);
            } else if (aperture->type() == Gerber::Circle) {
                drillDiameter = aperture->apertureSize();
            }

            data.emplace_back(std::move(name), drawApertureIcon(aperture.data()), dCode, drillDiameter);
            for (const GraphicObject* go : cacheApertures[dCode])
                giPeview[dCode].emplace_back(std::make_shared<DrillPrGI>(go, dCode, data.back()));
        }
    }

    return giPeview;
}

} // namespace Gerber
