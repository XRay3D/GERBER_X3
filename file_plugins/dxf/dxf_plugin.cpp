// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "dxf_plugin.h"
#include "dxf_file.h"
#include "dxf_node.h"
#include "dxf_settingstab.h"

#include "entities/dxf_allentities.h"
#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "section/dxf_headerparser.h"
#include "section/dxf_tables.h"
//#include "section/dxf_classes.h"
//#include "section/dxf_objects.h"
//#include "section/dxf_thumbnailimage.h"

#include "tables/dxf_layer.h"

#include "ft_view.h"

#include <QtWidgets>
#include <drillpreviewgi.h>

namespace Dxf {

Plugin::Plugin(QObject* parent)
    : QObject(parent)
{
}

FileInterface* Plugin::parseFile(const QString& fileName, int type_)
{
    if (type_ != type())
        return nullptr;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;

    m_file = new File;
    m_file->setFileName(fileName);

    int line = 1;

    Codes codes;
    codes.reserve(10000);

    QTextStream in(&file);
    //    in.setCodec("Windows-1251");

    //    in.setAutoDetectUnicode(true);

    auto getCode = [&in, &codes, &line, this] {
        // Code
        QString strCode(in.readLine());
        m_file->lines().push_back(strCode);
        bool ok;
        auto code(strCode.toInt(&ok));
        if (!ok)
            throw QString("Unknown code: raw str %1, line %2!").arg(strCode).arg(line);
        // Value
        QString strValue(in.readLine());
        m_file->lines().push_back(strValue);
        int multi = 0;
        while (strValue.endsWith("\\P")) {
            m_file->lines().push_back(in.readLine());
            strValue.append("\n" + m_file->lines().back());
            ++multi;
        }
        codes.emplace_back(code, strValue, line);
        line += 2 + multi;
        return *(codes.end() - 1);
    };

    try {
        int progress = 0;
        //int progressCtr = 0;
        do {
            if (auto code = getCode(); code.code() == 0 && code == "SECTION")
                ++progress;
        } while (!in.atEnd() || *(codes.end() - 1) != "EOF");
        codes.shrink_to_fit();
        file.close();

        //emit fileProgress(m_file->shortName(), progress, progressCtr);

        for (auto it = codes.begin(), from = codes.begin(), to = codes.begin(); it != codes.end(); ++it) {
            if (*it == "SECTION")
                from = it;
            if (auto it_ = it + 1; *it == "ENDSEC" && (*it_ == "SECTION" || *it_ == "EOF")) {
                //emit fileProgress(m_file->shortName(), 0, progressCtr++);
                to = it;
                const auto type = SectionParser::toType(*(from + 1));
                switch (type) {
                case SectionParser::HEADER:
                    m_file->m_sections[type] = new SectionHEADER(m_file, from, to);
                    break;
                case SectionParser::CLASSES:
                    //dxfFile()->m_sections[type] = new SectionCLASSES(dxfFile(), from, to);
                    break;
                case SectionParser::TABLES:
                    m_file->m_sections[type] = new SectionTABLES(m_file, from, to);
                    break;
                case SectionParser::BLOCKS:
                    m_file->m_sections[type] = new SectionBLOCKS(m_file, from, to);
                    break;
                case SectionParser::ENTITIES:
                    m_file->m_sections[type] = new SectionENTITIES(m_file, from, to);
                    break;
                case SectionParser::OBJECTS:
                    //dxfFile()->m_sections[type] = new SectionOBJECTS(dxfFile(), from, to);
                    break;
                case SectionParser::THUMBNAILIMAGE:
                    //dxfFile()->m_sections[type] = new SectionTHUMBNAILIMAGE(dxfFile(), from, to);
                    break;
                default:
                    throw QString("Unknowh Section!");
                    break;
                }
                if (m_file->m_sections.contains(type))
                    m_file->m_sections[type]->parse();
            }
        }
        if (m_file->m_sections.size() == 0) {
            delete m_file;
            m_file = nullptr;
        } else {
            //emit fileProgress(m_file->shortName(), 1, 1);
            emit fileReady(m_file);
        }
    } catch (const QString& wath) {
        qWarning() << "exeption QString:" << wath;
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), wath);
        delete m_file;
        return nullptr;
    } catch (const std::exception& e) {
        qWarning() << "exeption:" << e.what();
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString(e.what()));
        delete m_file;
        return nullptr;
    } catch (...) {
        qWarning() << "exeption:" << errno;
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString::number(errno));
        delete m_file;
        return nullptr;
    }
    return m_file;
}

QIcon Plugin::drawDrillIcon(QColor color)
{
    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(QRect(0, 0, IconSize - 1, IconSize - 1));
    return QIcon(pixmap);
}

bool Plugin::thisIsIt(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;
    QTextStream in(&file);
    do {
        QString line(in.readLine());
        if (line.toInt() == 999) {
            line = in.readLine();
            line = in.readLine();
        }
        if (line.toInt() != 0)
            break;
        if (line = in.readLine(); line != "SECTION")
            break;
        if (line = in.readLine(); line.toInt() != 2)
            break;
        if (line = in.readLine(); line != "HEADER")
            break;
        return true;
    } while (false);
    return false;
}

QObject* Plugin::getObject() { return this; }

int Plugin::type() const { return int(FileType::Dxf); }

QString Plugin::folderName() const { return tr("Dxf Files"); }

FileInterface* Plugin::createFile() { return new File(); }

QJsonObject Plugin::info() const
{
    return QJsonObject {
        { "Name", "Dxf" },
        { "Version", "1.0" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "Opening DXF Files" }
    };
}

SettingsTabInterface* Plugin::createSettingsTab(QWidget* parent)
{
    auto settingsTab = new SettingsTab(parent);
    settingsTab->setWindowTitle("DXF");
    return settingsTab;
}

void Plugin::updateFileModel(FileInterface* file)
{
    const auto fm = App::fileModel();
    const QModelIndex& fileIndex(file->node()->index());
    const QModelIndex index = fm->createIndex_(0, 0, fileIndex.internalId());
    // clean before insert new layers
    if (int count = fm->getItem(fileIndex)->childCount(); count) {
        fm->beginRemoveRows_(index, 0, count - 1);
        auto item = fm->getItem(index);
        do {
            item->remove(--count);
        } while (count);
        fm->endRemoveRows_();
    }
    Dxf::Layers layers;
    for (auto& [name, layer] : reinterpret_cast<File*>(file)->layers()) {
        //        qDebug() << name << layer;
        if (!layer->isEmpty())
            layers[name] = layer;
    }
    fm->beginInsertRows_(index, 0, int(layers.size() - 1));
    for (auto& [name, layer] : layers) {
        //        qDebug() << name << layer;
        fm->getItem(index)->addChild(new Dxf::NodeLayer(name, layer));
    }
    fm->endInsertRows_();
}

class DrillPrGI final : public AbstractDrillPrGI {
    const GraphicObject& go;
    const Circle* circle;
    const LwPolyline* lwpline;

public:
    explicit DrillPrGI(const GraphicObject& go, Row& row)
        : AbstractDrillPrGI(row)
        , go(go)
        , circle(dynamic_cast<const Circle*>(go.entity()))
        , lwpline(dynamic_cast<const LwPolyline*>(go.entity()))
    {
        m_sourceDiameter = circle ? circle->radius * 2
                                  : (QLineF(lwpline->poly.front(), lwpline->poly.back()).length() + lwpline->constantWidth) * go.scaleX();

        m_sourcePath = drawDrill();
        m_type = GiType::PrApetrure;
    }

private:
    QPainterPath drawDrill() const
    {
        QPainterPath painterPath;
        painterPath.addEllipse(circle ? circle->centerPoint
                                      : go.pos(),
            m_sourceDiameter * 0.5, m_sourceDiameter * 0.5);
        return painterPath;
    }

    Paths offset(const Path& path, double offset) const
    {
        ClipperOffset cpOffset;
        cpOffset.AddPath(path, jtRound, etOpenRound);
        Paths tmpPpaths;
        cpOffset.Execute(tmpPpaths, offset * 0.5 * uScale);
        for (Path& path : tmpPpaths)
            path.push_back(path.front());
        return tmpPpaths;
    }

    // AbstractDrillPrGI interface
public:
    void updateTool() override
    {
        row.toolId > -1 ? colorState |= Tool
                        : colorState &= ~Tool;
        m_toolPath = {};
        changeColor();
    }
    IntPoint pos() const override { return circle ? circle->centerPoint
                                                  : go.pos(); }
    Paths paths() const override
    {
        Paths paths { go.path() };
        return ReversePaths(paths);
    }
    bool fit(double depth) override { return m_sourceDiameter >= App::toolHolder().tool(row.toolId).getDiameter(depth); }
};

DrillPreviewGiMap Plugin::createDrillPreviewGi(FileInterface* file, mvector<Row>& data)
{
    auto const dxfFile = static_cast<File*>(file);
    DrillPreviewGiMap giPeview;

    int ctr {};
    for (auto& [key, lay] : dxfFile->layers())
        if (lay->isVisible()) {
            for (const auto& go : lay->graphicObjects())
                if (auto circle = dynamic_cast<const Circle*>(go.entity()); circle)
                    ++ctr;
                else if (auto lwp = dynamic_cast<const LwPolyline*>(go.entity()); lwp
                         && lwp->poly.size() == 2
                         && lwp->poly.front().bulge == 1.0
                         && lwp->poly.back().bulge == 1.0
                         && lwp->polylineFlag == 1)
                    ++ctr;
        }
    data.reserve(ctr);

    for (auto& [key, lay] : dxfFile->layers()) {
        std::map<double, mvector<const GraphicObject*>> cacheHoles;
        if (lay->isVisible())
            for (const auto& go : lay->graphicObjects()) {

                if (auto circle = dynamic_cast<const Circle*>(go.entity()); circle) {
                    cacheHoles[circle->radius * 2.0].push_back(&go);
                } else if (auto lwp = dynamic_cast<const LwPolyline*>(go.entity()); lwp
                           && lwp->poly.size() == 2
                           && lwp->poly[0].bulge == 1.0
                           && lwp->poly[1].bulge == 1.0
                           //                       && lwp->poly.front() == lwp->poly.back()
                           && lwp->polylineFlag == 1) {
                    cacheHoles[(QLineF(lwp->poly.front(), lwp->poly.back()).length() + lwp->constantWidth) * go.scaleX()].push_back(&go);
                }
            }

        for (auto [diameter, gos] : cacheHoles) {
            QString name(tr("Ø%1mm").arg(diameter));
            int id = static_cast<int>(data.size());
            data.emplace_back(std::move(name), drawDrillIcon(lay->color()), id, diameter);
            for (const auto go : gos) {
                giPeview[id].emplace_back(std::make_shared<DrillPrGI>(*go, data.back()));
            }
        }
    }

    return giPeview;
}

void Plugin::addToDrillForm(FileInterface* file, QComboBox* cbx)
{
    int ctr {};
    for (auto& [key, lay] : static_cast<File*>(file)->layers())
        if (lay->isVisible()) {
            for (const auto& go : lay->graphicObjects())
                if (auto circle = dynamic_cast<const Circle*>(go.entity()); circle) {
                    ++ctr;
                    break;
                } else if (auto lwp = dynamic_cast<const LwPolyline*>(go.entity()); lwp
                           && lwp->poly.size() == 2
                           && lwp->poly[0].bulge == 1.0
                           && lwp->poly[1].bulge == 1.0
                           //                       && lwp->poly.front() == lwp->poly.back()
                           && lwp->polylineFlag == 1) {
                    ++ctr;
                    break;
                }
        }
    if (ctr) {
        cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
        cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("crosshairs"));
        cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
    }
}

}
