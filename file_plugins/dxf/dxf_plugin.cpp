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
    QFile file_(fileName);
    if (!file_.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;

    file = new File;
    file->setFileName(fileName);

    int line = 1;

    Codes codes;
    codes.reserve(10000);

    QTextStream in(&file_);
    in.setAutoDetectUnicode(true);

    auto getCode = [&in, &codes, &line, this] {
        // Code
        QString strCode(in.readLine());
        file->lines().push_back(strCode);
        bool ok;
        auto code(strCode.toInt(&ok));
        if (!ok)
            throw QString("Unknown code: raw str %1, line %2!").arg(strCode).arg(line);
        // Value
        QString strValue(in.readLine());
        file->lines().push_back(strValue);
        int multi = 0;
        while (strValue.endsWith("\\P")) {
            file->lines().push_back(in.readLine());
            strValue.append("\n" + file->lines().back());
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
        file_.close();

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
                    file->m_sections[type] = new SectionHEADER(file, from, to);
                    break;
                case SectionParser::CLASSES:
                    //dxfFile()->m_sections[type] = new SectionCLASSES(dxfFile(), from, to);
                    break;
                case SectionParser::TABLES:
                    file->m_sections[type] = new SectionTABLES(file, from, to);
                    break;
                case SectionParser::BLOCKS:
                    file->m_sections[type] = new SectionBLOCKS(file, from, to);
                    break;
                case SectionParser::ENTITIES:
                    file->m_sections[type] = new SectionENTITIES(file, from, to);
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
                if (file->m_sections.contains(type))
                    file->m_sections[type]->parse();
            }
        }
        if (file->m_sections.size() == 0) {
            delete file;
            file = nullptr;
        } else {
            //emit fileProgress(m_file->shortName(), 1, 1);
            emit fileReady(file);
        }
    } catch (const QString& wath) {
        qWarning() << "exeption QString:" << wath;
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), wath);
        delete file;
        return nullptr;
    } catch (const std::exception& e) {
        qWarning() << "exeption:" << e.what();
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString(e.what()));
        delete file;
        return nullptr;
    } catch (...) {
        qWarning() << "exeption:" << errno;
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString::number(errno));
        delete file;
        return nullptr;
    }
    return file;
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

std::pair<SettingsTabInterface*, QString> Plugin::createSettingsTab(QWidget* parent)
{
    return { new SettingsTab(parent), "DXF" };
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
    const Circle* c;

public:
    explicit DrillPrGI(const GraphicObject& go, Row& row)
        : AbstractDrillPrGI(row)
        , go(go)
        , c(static_cast<const Circle*>(go.entity()))
    {
        m_sourceDiameter = c->radius * 2;
        m_sourcePath = drawDrill();
        m_type = GiType::PrDrill;
    }

private:
    QPainterPath drawDrill() const
    {
        QPainterPath painterPath;
        painterPath.addEllipse(c->centerPoint, c->radius, c->radius);
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
            path.push_back(path.front());
        return tmpPpaths;
    }

    // AbstractDrillPrGI interface
public:
    void updateTool() override
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
                offset.AddPath(go.paths().front(), jtRound, etOpenRound);
                offset.Execute(tmpPpath, diameter * 0.5 * uScale);

                for (Path& path : tmpPpath) {
                    path.push_back(path.front());
                    m_toolPath.addPolygon(path);
                }

                Path path(go.paths().front());

                if (path.size()) {
                    for (IntPoint& pt : path) {
                        m_toolPath.moveTo(pt - QPointF(0.0, lineKoeff));
                        m_toolPath.lineTo(pt + QPointF(0.0, lineKoeff));
                        m_toolPath.moveTo(pt - QPointF(lineKoeff, 0.0));
                        m_toolPath.lineTo(pt + QPointF(lineKoeff, 0.0));
                    }
                    m_toolPath.moveTo(path.front());
                    for (IntPoint& pt : path) {
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
    IntPoint pos() const override
    {
        return c->centerPoint;
    }
    Paths paths() const override
    {
        if (m_type == GiType::PrSlot)
            return go.paths();
        Paths paths(go.paths());
        return ReversePaths(paths);
    }
    bool fit(double depth) override { return m_sourceDiameter > App::toolHolder().tool(row.toolId).getDiameter(depth); }
};

DrillPreviewGiMap Plugin::createDrillPreviewGi(FileInterface* file, mvector<Row>& data)
{
    DrillPreviewGiMap giPeview;

    auto const dxfFile = static_cast<File*>(file);

    std::map<double, mvector<const GraphicObject*>> cacheHoles;

    for (auto& [key, lay] : dxfFile->layers()) {
        for (const auto& go : lay->graphicObjects()) {
            if (auto circle = dynamic_cast<const Circle*>(go.entity()); circle) {
                cacheHoles[circle->radius * 2].push_back(&go);
                qDebug() << circle->radius << circle->centerPoint << circle->id << go.entityId();
            }
        }
    }

    int ctr {};
    for (auto [diameter, gos] : cacheHoles) {
        QString name(tr("Ø%1mm").arg(diameter));
        data.emplace_back(std::move(name), drawDrillIcon(), diameter, ++ctr);
        for (const GraphicObject* go : gos) {
            giPeview[ctr].emplace_back(std::make_shared<DrillPrGI>(*go, data.back()));
        }
    }

    return giPeview;
}

void Plugin::addToDrillForm(FileInterface* file, QComboBox* cbx)
{
    cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
    cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("crosshairs"));
    cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
}

}
