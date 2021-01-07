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
#include "explugin.h"
#include "exfile.h"
#include "exnode.h"
#include "extypes.h"

#include "app.h"
#include "doublespinbox.h"
#include "drillitem.h"
#include "drillpreviewgi.h"
#include "interfaces/file.h"
#include "treeview.h"

#include <QtWidgets>

#include "leakdetector.h"

namespace Excellon {

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

//const int id1 = qRegisterMetaType<Gerber::File*>("Gerber::GFile*");

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
    Parser::parseFile(fileName);
    return Parser::file;
}

QIcon Plugin::drawDrillIcon()
{
    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawEllipse(QRect(0, 0, IconSize - 1, IconSize - 1));
    return QIcon(pixmap);
}

bool Plugin::thisIsIt(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in(&file);
    QString line;
    const QRegExp match("^T([0]?[0-9]{1})[FSC]((\\d*\\.?\\d+))?.*$");
    const QRegExp match2(".*Holesize.*");
    while (in.readLineInto(&line)) {
        if (match.exactMatch(line))
            return true;
        if (match2.exactMatch(line))
            return true;
    }

    return false;
}

QObject* Plugin::getObject() { return this; }

int Plugin::type() const { return int(FileType::Excellon); }

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

//std::pair<SettingsTabInterface*, QString> Plugin::createSettingsTab(QWidget* parent)
//{
//    class Tab : public SettingsTabInterface, Settings {
//        QCheckBox* chbxCleanPolygons;
//        QCheckBox* chbxSkipDuplicates;
//        QCheckBox* chbxSimplifyRegions;
//        DoubleSpinBox* dsbxCleanPolygonsDist;

//    public:
//        Tab(QWidget* parent = nullptr)
//            : SettingsTabInterface(parent)
//        {
//            setObjectName(QString::fromUtf8("tabDxf"));
//            auto verticalLayout = new QVBoxLayout(this);
//            verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
//            verticalLayout->setContentsMargins(6, 6, 6, 6);

//            auto groupBox = new QGroupBox(this);
//            groupBox->setObjectName(QString::fromUtf8("groupBox"));
//            auto verticalLayout2 = new QVBoxLayout(groupBox);
//            verticalLayout2->setObjectName(QString::fromUtf8("verticalLayout2"));
//            verticalLayout2->setContentsMargins(6, 9, 6, 6);

//            chbxCleanPolygons = new QCheckBox(groupBox);
//            chbxCleanPolygons->setObjectName(QString::fromUtf8("chbxCleanPolygons"));
//            verticalLayout2->addWidget(chbxCleanPolygons);

//            dsbxCleanPolygonsDist = new DoubleSpinBox(groupBox);
//            dsbxCleanPolygonsDist->setObjectName(QString::fromUtf8("dsbxCleanPolygonsDist"));
//            dsbxCleanPolygonsDist->setRange(0.0001, 1.0);
//            dsbxCleanPolygonsDist->setSingleStep(0.001);
//            dsbxCleanPolygonsDist->setDecimals(4);
//            verticalLayout2->addWidget(dsbxCleanPolygonsDist);

//            chbxSkipDuplicates = new QCheckBox(groupBox);
//            chbxSkipDuplicates->setObjectName(QString::fromUtf8("chbxSkipDuplicates"));
//            verticalLayout2->addWidget(chbxSkipDuplicates);

//            chbxSimplifyRegions = new QCheckBox(groupBox);
//            chbxSimplifyRegions->setObjectName(QString::fromUtf8("chbxSimplifyRegions"));
//            verticalLayout2->addWidget(chbxSimplifyRegions);

//            verticalLayout->addWidget(groupBox);
//            auto verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
//            verticalLayout->addItem(verticalSpacer);

//            groupBox->setTitle(QApplication::translate("SettingsDialog", "Gerber", nullptr));
//            chbxCleanPolygons->setText(QApplication::translate("SettingsDialog", "Cleaning Polygons", nullptr));
//            chbxSkipDuplicates->setText(QApplication::translate("SettingsDialog", "Skip duplicates", nullptr));
//            chbxSimplifyRegions->setText(QApplication::translate("SettingsDialog", "Simplify Regions", nullptr));
//        }
//        virtual ~Tab() override { qDebug(__FUNCTION__); }

//        virtual void readSettings(MySettings& settings) override
//        {
//            settings.beginGroup("Gerber");
//            m_cleanPolygons = settings.getValue(chbxCleanPolygons, m_cleanPolygons);
//            m_cleanPolygonsDist = settings.getValue(dsbxCleanPolygonsDist, m_cleanPolygonsDist);
//            m_simplifyRegions = settings.getValue(chbxSimplifyRegions, m_simplifyRegions);
//            m_skipDuplicates = settings.getValue(chbxSkipDuplicates, m_skipDuplicates);
//            settings.endGroup();
//        }
//        virtual void writeSettings(MySettings& settings) override
//        {
//            settings.beginGroup("Gerber");
//            m_cleanPolygons = settings.setValue(chbxCleanPolygons);
//            m_cleanPolygonsDist = settings.setValue(dsbxCleanPolygonsDist);
//            m_simplifyRegions = settings.setValue(chbxSimplifyRegions);
//            m_skipDuplicates = settings.setValue(chbxSkipDuplicates);
//            settings.endGroup();
//        }
//    };
//    return { new Tab(parent), "Gerber X3" };
//}

void Plugin::addToDrillForm(FileInterface* file, QComboBox* cbx)
{
    cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
    cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("drill-path"));
    cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
}

class DrillPrGI final : public AbstractDrillPrGI {
public:
    explicit DrillPrGI(const Excellon::Hole* hole, Row& row)
        : AbstractDrillPrGI(row)
        , hole(hole)
    {
        m_sourceDiameter = hole->state.currentToolDiameter();
        m_sourcePath = hole->state.path.isEmpty() ? drawDrill() : drawSlot();
        m_type = hole->state.path.isEmpty() ? GiType::PrDrill : GiType::PrSlot;
    }

private:
    QPainterPath drawDrill() const
    {
        QPainterPath painterPath;
        const double radius = hole->state.currentToolDiameter() * 0.5;
        painterPath.addEllipse(hole->state.offsetedPos(), radius, radius);
        return painterPath;
    }

    QPainterPath drawSlot() const
    {
        QPainterPath painterPath;
        for (Path& path : offset(hole->item->paths().first(), hole->state.currentToolDiameter()))
            painterPath.addPolygon(path);
        return painterPath;
    }

    Paths offset(const Path& path, double offset) const
    {
        ClipperOffset cpOffset;
        // cpOffset.AddPath(path, jtRound, etClosedLine);
        cpOffset.AddPath(path, jtRound, etOpenRound);
        Paths tmpPpaths;
        cpOffset.Execute(tmpPpaths, offset * 0.5 * uScale);
        for (Path& path : tmpPpaths)
            path.append(path.first());
        return tmpPpaths;
    }

    const Hole* hole;

    // AbstractDrillPrGI interface
public:
    void
    updateTool() override
    {
        if (row.toolId > -1) {
            colorState |= Tool;
            if (m_type == GiType::PrSlot) {
                m_toolPath = {};

                auto& tool(App::toolHolder().tool(row.toolId));
                const double diameter = tool.getDiameter(tool.getDepth());
                const double lineKoeff = diameter * 0.7;

                Paths tmpPpath;

                ClipperOffset offset;
                offset.AddPath(hole->item->paths().first(), jtRound, etOpenRound);
                offset.Execute(tmpPpath, diameter * 0.5 * uScale);

                for (Path& path : tmpPpath) {
                    path.push_back(path.first());
                    m_toolPath.addPolygon(path);
                }

                Path path(hole->item->paths().first());

                if (path.size()) {
                    for (Point64& pt : path) {
                        m_toolPath.moveTo(pt - QPointF(0.0, lineKoeff));
                        m_toolPath.lineTo(pt + QPointF(0.0, lineKoeff));
                        m_toolPath.moveTo(pt - QPointF(lineKoeff, 0.0));
                        m_toolPath.lineTo(pt + QPointF(lineKoeff, 0.0));
                    }
                    m_toolPath.moveTo(path.first());
                    for (Point64& pt : path) {
                        m_toolPath.lineTo(pt);
                    }
                }
            }
        } else {
            colorState &= ~Tool;
            m_toolPath = {};
        }

        changeColor();
    }
    Point64 pos() const override { return hole->state.offsetedPos(); }
    Paths paths() const override
    {
        if (m_type == GiType::PrSlot)
            return hole->item->paths();
        Paths paths(hole->item->paths());
        return ReversePaths(paths);
    }
    bool fit(double depth) override
    {
        return m_sourceDiameter > App::toolHolder().tool(row.toolId).getDiameter(depth);
    }
};

DrillPreviewGiMap Plugin::createDrillPreviewGi(FileInterface* file, mvector<Row>& data)
{
    DrillPreviewGiMap giPeview;

    auto const exFile = reinterpret_cast<File*>(file);

    std::map<int, mvector<const Excellon::Hole*>> cacheHoles;
    for (const Excellon::Hole& hole : *exFile)
        cacheHoles[hole.state.tCode] << &hole;

    data.reserve(cacheHoles.size());// !!! reserve для отсутствия реалокаций, так как DrillPrGI хранит ссылки на него !!!

    for (auto [toolNum, diameter] : exFile->tools()) {
        QString name(tr("Tool Ø%1mm").arg(diameter));
        data.emplace_back(std::move(name), drawDrillIcon(), toolNum, diameter);
        for (const Excellon::Hole* hole : cacheHoles[toolNum]) {
            if (!hole->state.path.isEmpty())
                data.back().isSlot = true;
            giPeview[toolNum].emplace_back(std::make_shared<DrillPrGI>(hole, data.back()));
        }
    }
    return giPeview;
}

} // namespace Gerber
