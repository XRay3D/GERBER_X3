// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "gcfile.h"

#include "forms/gcodepropertiesform.h"

#include "gi/aperturepathitem.h"
#include "gi/drillitem.h"
#include "gi/gerberitem.h"
#include "gi/pathitem.h"
#include "graphicsview.h"
#include "point.h"
#include "project.h"
#include "settings.h"
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QTextStream>
#include <gi/itemgroup.h>

#include "leakdetector.h"

namespace GCode {

File::File()
    : GCUtils(m_gcp)
{
}

File::File(const Pathss& toolPathss, const GCodeParams& gcp, const Paths& pocketPaths)
    : GCUtils(m_gcp)
    , m_pocketPaths(pocketPaths)
    , m_toolPathss(toolPathss)
    , m_gcp(gcp)
{
    createGi();
    if (gcp.tools.first().diameter()) {
        qDebug(Q_FUNC_INFO);
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

bool File::save(const QString& name)
{
    if (name.isEmpty())
        return false;

    initSave();
    addInfo();
    statFile();
    genGcodeAndTile();
    endFile();

    setLastDir(name);
    m_name = name;
    QFile file(m_name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString str;
        for (QString& s : m_lines) {
            if (!s.isEmpty())
                str.append(s);
            if (!str.endsWith('\n'))
                str.append("\n");
        }
        out << str;
    } else
        return false;
    file.close();
    return true;
}

void File::saveDrill(const QPointF& offset)
{
    QPolygonF path(normalizedPaths(offset, m_toolPathss.first()).first());

    const QVector<double> depths(getDepths());

    for (QPointF& point : path) {
        startPath(point);
        int i = 0;
        while (true) {
            m_lines.append(formated({ g1(), z(depths[i]), feed(plungeRate()) }));
            if (++i == depths.count())
                break;
            m_lines.append(formated({ g0(), z(0.0) }));
        }
        endPath();
    }
}

void File::saveLaserPocket(const QPointF& offset)
{
    saveLaserProfile(offset);
    //    m_lines.append(GlobalSettings::gcLaserDynamOn());

    //    QVector<QVector<QPolygonF>> toolPathss(normalizedPathss(offset));

    //    for (QVector<QPolygonF>& paths : toolPathss) {
    //        startPath(paths.first().first());
    //        bool skip = true;
    //        for (auto& path : paths) {
    //            for (QPointF& point : path) {
    //                if (skip)
    //                    skip = false;
    //                else
    //                    m_lines.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
    //            }
    //        }
    //        endPath();
    //    }
}

void File::saveMillingPocket(const QPointF& offset)
{
    m_lines.append(GlobalSettings::gcSpindleOn());

    QVector<QVector<QPolygonF>> toolPathss(normalizedPathss(offset));

    const QVector<double> depths(getDepths());

    for (QVector<QPolygonF>& paths : toolPathss) {
        startPath(paths.first().first());
        for (int i = 0; i < depths.count(); ++i) {
            m_lines.append(formated({ g1(), z(depths[i]), feed(plungeRate()) }));
            bool skip = true;
            for (auto& path : paths) {
                for (QPointF& point : path) {
                    if (skip)
                        skip = false;
                    else
                        m_lines.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
                }
            }
            for (int j = paths.size() - 2; j >= 0 && i < depths.count() - 1; --j) {
                QPointF& point = paths[j].last();
                m_lines.append(formated({ g0(), x(point.x()), y(point.y()) }));
            }
            if (paths.size() > 1 && i < (depths.count() - 1))
                m_lines.append(formated({ g0(), x(paths.first().first().x()), y(paths.first().first().y()) }));
        }
        endPath();
    }
}

void File::saveMillingProfile(const QPointF& offset)
{
    if (m_gcp.gcType == Raster) {
        saveMillingRaster(offset);
        return;
    }

    QVector<QVector<QPolygonF>> pathss(normalizedPathss(offset));
    const QVector<double> depths(getDepths());

    for (auto& paths : pathss) {
        for (int i = 0; i < depths.count(); ++i) {
            for (int j = 0; j < paths.size(); ++j) {
                QPolygonF& path = paths[j];
                if (path.first() == path.last()) { // make complete depth and remove from worck
                    startPath(path.first());
                    for (int k = 0; k < depths.count(); ++k) {
                        m_lines.append(formated({ g1(), z(depths[k]), feed(plungeRate()) }));
                        m_lines.append(savePath(path, spindleSpeed()));
                        //                    bool skip = true;
                        //                    for (QPointF& point : path) {
                        //                        if (skip)
                        //                            skip = false;
                        //                        else
                        //                            m_lines.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
                        //                    }
                    }
                    endPath();
                    paths.remove(j--);
                } else {
                    startPath(path.first());
                    m_lines.append(formated({ g1(), z(depths[i]), feed(plungeRate()) }));
                    m_lines.append(savePath(path, spindleSpeed()));
                    //                    bool skip = true;
                    //                    for (QPointF& point : path) {
                    //                        if (skip)
                    //                            skip = false;
                    //                        else
                    //                            m_lines.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
                    //                    }
                    endPath();
                }
            }
        }
    }
}

void File::saveLaserProfile(const QPointF& offset)
{
    m_lines.append(GlobalSettings::gcLaserDynamOn());

    QVector<QVector<QPolygonF>> pathss(normalizedPathss(offset));

    for (auto& paths : pathss) {
        for (auto& path : paths) {
            startPath(path.first());
            m_lines.append(savePath(path, spindleSpeed()));
            //            bool skip = true;
            //            for (QPointF& point : path) {
            //                if (skip)
            //                    skip = false;
            //                else
            //                    m_lines.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()), speed(spindleSpeed()) }));
            //            }
            endPath();
        }
    }
}

void File::saveMillingRaster(const QPointF& offset)
{
    m_lines.append(GlobalSettings::gcSpindleOn());

    QVector<QVector<QPolygonF>> pathss(normalizedPathss(offset));
    const QVector<double> depths(getDepths());

    for (auto& paths : pathss) {
        for (int i = 0; i < depths.count(); ++i) {
            for (auto& path : paths) {
                startPath(path.first());
                m_lines.append(formated({ g1(), z(depths[i]), feed(plungeRate()) }));
                m_lines.append(savePath(path, spindleSpeed()));
                //                bool skip = true;
                //                for (QPointF& point : path) {
                //                    if (skip)
                //                        skip = false;
                //                    else
                //                        m_lines.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
                //                }
                endPath();
            }
        }
    }
}

void File::saveLaserHLDI(const QPointF& offset)
{
    m_lines.append(GlobalSettings::gcLaserConstOn());

    QVector<QVector<QPolygonF>> pathss(normalizedPathss(offset));

    int i = 0;

    m_lines.append(formated({ g0(), x(pathss.first().first().first().x()), y(pathss.first().first().first().y()), z(0.0) }));

    for (QPolygonF& path : pathss.first()) {
        if (i++ % 2) {
            m_lines.append(savePath(path, spindleSpeed()));
            //            bool skip = true;
            //            for (QPointF& point : path) {
            //                if (skip)
            //                    skip = false;
            //                else
            //                    m_lines.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()), speed(spindleSpeed()) }));
            //            }
        } else {
            m_lines.append(savePath(path, 0));
            //            bool skip = true;
            //            for (QPointF& point : path) {
            //                if (skip)
            //                    skip = false;
            //                else
            //                    m_lines.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()), speed(static_cast<int>(0)) }));
            //            }
        }
    }
    if (pathss.size() > 1) {
        m_lines.append(GlobalSettings::gcLaserDynamOn());
        for (QPolygonF& path : pathss.last()) {
            startPath(path.first());
            m_lines.append(savePath(path, spindleSpeed()));
            //            bool skip = true;
            //            for (QPointF& point : path) {
            //                if (skip)
            //                    skip = false;
            //                else
            //                    m_lines.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
            //            }
            endPath();
        }
    }
}

const GCodeParams& File::gcp() const { return m_gcp; }

QVector<QVector<QPolygonF>> File::normalizedPathss(const QPointF& offset)
{
    QVector<QVector<QPolygonF>> pathss;
    pathss.reserve(m_toolPathss.size());
    for (const Paths& paths : m_toolPathss) {
        pathss.append(normalizedPaths(offset, paths));
        //        pathss.append(toQPolygons(paths));
    }

    //    for (QVector<QPolygonF>& paths : pathss)
    //        for (QPolygonF& path : paths)
    //            path.translate(offset);

    //    if (m_side == Bottom) {
    //        const double k = Pin::minX() + Pin::maxX();
    //        for (auto& paths : pathss) {
    //            for (auto& path : paths) {
    //                if (toolType() != Tool::Laser)
    //                    std::reverse(path.begin(), path.end());
    //                for (QPointF& point : path) {
    //                    point.rx() = -point.x() + k;
    //                }
    //            }
    //        }
    //    }

    //    for (auto& paths : pathss) {
    //        for (auto& path : paths) {
    //            for (QPointF& point : path) {
    //                point -= Marker::get(Marker::Zero)->pos();
    //            }
    //        }
    //    }

    return pathss;
}

QVector<QPolygonF> File::normalizedPaths(const QPointF& offset, const Paths& paths_)
{
    QVector<QPolygonF> paths(paths_.isEmpty() ? toQPolygons(m_toolPathss.first()) : toQPolygons(paths_));

    for (QPolygonF& path : paths)
        path.translate(offset);

    if (m_side == Bottom) {
        const double k = Pin::minX() + Pin::maxX();
        for (auto& path : paths) {
            if (toolType() != Tool::Laser)
                std::reverse(path.begin(), path.end());
            for (QPointF& point : path) {
                point.rx() = -point.x() + k;
            }
        }
    }
    for (auto& path : paths) {
        for (QPointF& point : path) {
            point -= Marker::get(Marker::Zero)->pos();
        }
    }

    return paths;
}

void File::initSave()
{
    m_lines.clear();

    for (bool& fl : formatFlags)
        fl = false;

    const QString format(GlobalSettings::gcFormat());
    for (int i = 0; i < cmdList.size(); ++i) {
        const int index = format.indexOf(cmdList[i], 0, Qt::CaseInsensitive);
        if (index != -1) {
            formatFlags[i + AlwaysG] = format[index + 1] == '+';
            if ((index + 2) < format.size())
                formatFlags[i + SpaceG] = format[index + 2] == ' ';
        }
    }

    for (QString& str : lastValues)
        str.clear();

    setFeedRate(m_gcp.getTool().feedRate());
    setPlungeRate(m_gcp.getTool().plungeRate());
    setSpindleSpeed(m_gcp.getTool().spindleSpeed());
    setToolType(m_gcp.getTool().type());
}

void File::genGcodeAndTile()
{
    const QRectF rect = App::project()->worckRect();
    for (int x = 0; x < App::project()->stepsX(); ++x) {
        for (int y = 0; y < App::project()->stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project()->spaceX()) * x, (rect.height() + App::project()->spaceY()) * y);

            switch (m_gcp.gcType) {
            case Pocket:
                if (toolType() == Tool::Laser)
                    saveLaserPocket(offset);
                else
                    saveMillingPocket(offset);
                break;
            case Voronoi:
                if (toolType() == Tool::Laser) {
                    if (m_toolPathss.size() > 1)
                        saveLaserPocket(offset);
                    else
                        saveLaserProfile(offset);
                } else {
                    if (m_toolPathss.size() > 1)
                        saveMillingPocket(offset);
                    else
                        saveMillingProfile(offset);
                }
                break;
            case Profile:
            case Thermal:
            case Raster:
                if (toolType() == Tool::Laser)
                    saveLaserProfile(offset);
                else
                    saveMillingProfile(offset);
                break;
            case Drill:
                saveDrill(offset);
                break;
            case LaserHLDI:
                saveLaserHLDI(offset);
                break;
            default:
                break;
            }
            if (m_gcp.params.contains(GCodeParams::NotTile))
                return;
        }
    }
}

Tool File::getTool() const { return m_gcp.getTool(); }

void File::addInfo(bool fl)
{
    if (GlobalSettings::gcInfo() || fl) {
        m_lines.append(QString(";\tName:\t%1").arg(shortName()));
        m_lines.append(QString(";\tTool:\t%1").arg(m_gcp.getTool().name()));
        m_lines.append(QString(";\tDepth:\t%1").arg(m_gcp.getDepth()));
        m_lines.append(QString(";\tSide:\t%1").arg(QStringList { "Top", "Bottom" }[side()]));
    }
}

GCodeType File::gtype() const { return m_gcp.gcType; }

void File::startPath(const QPointF& point)
{
    if (toolType() == Tool::Laser) {
        m_lines.append(formated({ g0(), x(point.x()), y(point.y()), speed(0) })); //start xy
        //        m_gCodeText.append(formated({ g1(), speed(spindleSpeed) }));
    } else {
        m_lines.append(formated({ g0(), x(point.x()), y(point.y()), speed(spindleSpeed()) })); //start xy
        m_lines.append(formated({ g0(), z(GCodePropertiesForm::plunge) })); //start z
        //        lastValues[AlwaysF].clear();
    }
}

void File::endPath()
{
    if (toolType() == Tool::Laser) {
        //
    } else {
        m_lines.append(formated({ g0(), z(GCodePropertiesForm::clearence) }));
    }
}

void File::statFile()
{
    if (toolType() == Tool::Laser) {
        QString str(GlobalSettings::gcLaserStart()); //"G21 G17 G90"); //G17 XY plane
        m_lines.append(str);
        m_lines.append(formated({ g0(), z(0) })); // Z0 for visible in Candle
    } else {
        QString str(GlobalSettings::gcStart()); //"G21 G17 G90"); //G17 XY plane
        str.replace(QRegExp("S\\?"), formated({ speed(spindleSpeed()) }));
        m_lines.append(str);
        m_lines.append(formated({ g0(), z(GCodePropertiesForm::safeZ) })); //HomeZ
    }
}

void File::endFile()
{
    if (toolType() == Tool::Laser) {
        m_lines.append(GlobalSettings::gcSpindleLaserOff());
        QPointF home(Marker::get(Marker::Home)->pos() - Marker::get(Marker::Zero)->pos());
        m_lines.append(formated({ g0(), x(home.x()), y(home.y()) })); //HomeXY
        m_lines.append(GlobalSettings::gcLaserEnd());
    } else {
        m_lines.append(formated({ g0(), z(GCodePropertiesForm::safeZ) })); //HomeZ
        QPointF home(Marker::get(Marker::Home)->pos() - Marker::get(Marker::Zero)->pos());
        m_lines.append(formated({ g0(), x(home.x()), y(home.y()) })); //HomeXY
        m_lines.append(GlobalSettings::gcEnd());
    }
    for (int i = 0; i < m_lines.size(); ++i) { // remove epty lines
        if (m_lines[i].isEmpty())
            m_lines.removeAt(i--);
    }
}

QList<QString> File::gCodeText() const { return m_lines; }

void File::createGiDrill()
{
    GraphicsItem* item;
    for (const IntPoint& point : m_toolPathss.first().first()) {
        item = new DrillItem(m_gcp.getTool().diameter(), this);
        item->setPos(point);
        item->setPenColor(&GlobalSettings::guiColor(Colors::ToolPath));
        item->setColorP(&GlobalSettings::guiColor(Colors::CutArea));
        itemGroup()->append(item);
    }
    item = new PathItem(m_toolPathss.first().first());
    item->setPenColor(&GlobalSettings::guiColor(Colors::G0));
    itemGroup()->append(item);
}

void File::createGiPocket()
{
    GraphicsItem* item;
    if (m_pocketPaths.size()) {
        //        {
        //            ClipperOffset offset(uScale);
        //            offset.AddPaths(m_pocketPaths, jtRound, etClosedPolygon);
        //            offset.Execute(m_pocketPaths, uScale * m_gcp.getToolDiameter() * 0.5);
        //        }
        item = new GiGerber(m_pocketPaths, nullptr);
        item->setPen(Qt::NoPen);
        item->setColorP(&GlobalSettings::guiColor(Colors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->append(item);
    }
    m_g0path.reserve(m_toolPathss.size());
    int i = 0;
    for (const Paths& paths : m_toolPathss) {
        int k = static_cast<int>((m_toolPathss.size() > 1) ? (300.0 / (m_toolPathss.size() - 1)) * i : 0);
        debugColor.append(QSharedPointer<QColor>(new QColor(QColor::fromHsv(k, 255, 255, 255))));

        for (const Path& path : paths) {
            item = new PathItem(path, this);
#ifdef QT_DEBUG
            item->setPenColor(*debugColor.last());
#else
            item->setPenColor(&GlobalSettings::guiColor(Colors::ToolPath));
#endif
            itemGroup()->append(item);
        }

        {
            Paths g1path;
            for (int j = 0; j < paths.count() - 1; ++j)
                g1path.append({ paths[j].last(), paths[j + 1].first() });
            item = new PathItem(g1path);
#ifdef QT_DEBUG
            debugColor.append(QSharedPointer<QColor>(new QColor(0, 0, 255)));
            item->setPenColor(*debugColor.last());
#else
            item->setPenColor(&GlobalSettings::guiColor(Colors::ToolPath));
#endif
            itemGroup()->append(item);
        }

        if (i < m_toolPathss.size() - 1) {
            m_g0path.append({ m_toolPathss[i].last().last(), m_toolPathss[++i].first().first() });
        }
    }
    item = new PathItem(m_g0path);
    item->setPenColor(&GlobalSettings::guiColor(Colors::G0));
    itemGroup()->append(item);
}

void File::createGiProfile()
{
    GraphicsItem* item;
    for (const Paths& paths : m_toolPathss) {
        item = new PathItem(paths, this);
        item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColor(&GlobalSettings::guiColor(Colors::CutArea));
        itemGroup()->append(item);
    }
    int i = 0;
    for (const Paths& paths : m_toolPathss) {
        item = new PathItem(m_toolPathss[i], this);
        item->setPenColor(&GlobalSettings::guiColor(Colors::ToolPath));
        itemGroup()->append(item);
        for (int j = 0; j < paths.count() - 1; ++j)
            m_g0path.append({ paths[j].last(), paths[j + 1].first() });
        if (i < m_toolPathss.size() - 1) {
            m_g0path.append({ m_toolPathss[i].last().last(), m_toolPathss[++i].first().first() });
        }
    }

    item = new PathItem(m_g0path);
    //    item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColor(&GlobalSettings::guiColor(Colors::G0));
    itemGroup()->append(item);
}

void File::createGiRaster()
{
    //        int k = static_cast<int>((m_toolPathss.size() > 1) ? (300.0 / (m_toolPathss.size() - 1)) * i : 0);
    //        QColor* c = new QColor;
    //        *c = QColor::fromHsv(k, 255, 255, 255);
    GraphicsItem* item;
    m_g0path.reserve(m_toolPathss.size());

    if (m_pocketPaths.size()) {
        item = new GiGerber(m_pocketPaths, nullptr);
        item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColor(&GlobalSettings::guiColor(Colors::CutArea));
        item->setColorP(&GlobalSettings::guiColor(Colors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->append(item);
    } else {
        for (const Paths& paths : m_toolPathss) {
            item = new PathItem(paths, this);
            item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColor(&GlobalSettings::guiColor(Colors::CutArea));
            itemGroup()->append(item);
        }
    }
    int i = 0;
    for (const Paths& paths : m_toolPathss) {
        item = new PathItem(paths, this);
        item->setPenColor(&GlobalSettings::guiColor(Colors::ToolPath));
        itemGroup()->append(item);
        for (int j = 0; j < paths.count() - 1; ++j)
            m_g0path.append({ paths[j].last(), paths[j + 1].first() });
        if (i < m_toolPathss.size() - 1) {
            m_g0path.append({ m_toolPathss[i].last().last(), m_toolPathss[++i].first().first() });
        }
    }
    item = new PathItem(m_g0path);
    item->setPenColor(&GlobalSettings::guiColor(Colors::G0));
    itemGroup()->append(item);
}

void File::createGiLaser()
{
    GraphicsItem* item;
    Paths paths;
    paths.reserve(m_toolPathss.first().size() / 2 + 1);
    m_g0path.reserve(paths.size());
    for (int i = 0; i < m_toolPathss.first().size(); ++i) {
        if (i % 2)
            paths.append(m_toolPathss.first()[i]);
        else
            m_g0path.append(m_toolPathss.first()[i]);
    }
    if (m_toolPathss.size() > 1) {
        paths.append(m_toolPathss[1]);
        m_g0path.append({ m_toolPathss[0].last().last(), m_toolPathss[1].first().first() });
        for (int i = 0; i < m_toolPathss[1].count() - 1; ++i)
            m_g0path.append({ m_toolPathss[1][i].last(), m_toolPathss[1][i + 1].first() });
    }

    item = new PathItem(paths, this);
    item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item->setPenColor(&GlobalSettings::guiColor(Colors::CutArea));
    itemGroup()->append(item);

    item = new PathItem(paths, this);
    item->setPenColor(&GlobalSettings::guiColor(Colors::ToolPath));
    itemGroup()->append(item);

    item = new PathItem(m_g0path);
    item->setPenColor(&GlobalSettings::guiColor(Colors::G0));
    itemGroup()->append(item);
}

void File::write(QDataStream& stream) const
{
    //    switch (Project::ver()) {
    //    case G2G_Ver_4:
    stream << m_gcp;
    stream << m_pocketPaths;
    stream << m_toolPathss;
    //    case G2G_Ver_3:
    //        [[fallthrough]];
    //    case G2G_Ver_2:
    //        [[fallthrough]];
    //    case  GiType::G2G_Ver_1::
    //    }
    //    stream << *static_cast<const AbstractFile*>(this);
    //_write(stream);
}

void File::read(QDataStream& stream)
{
    auto& gcp = *const_cast<GCodeParams*>(&m_gcp);
    switch (App::project()->ver()) {
    case ProVer_4:
        stream >> gcp;
        stream >> m_pocketPaths;
        stream >> m_toolPathss;
        break;
    case ProVer_3: {
        stream >> m_pocketPaths;
        stream >> gcp.gcType;
        stream >> m_toolPathss;
        gcp.tools.resize(1);
        stream >> gcp.tools.first();
        double depth;
        stream >> depth;
        gcp.params[GCodeParams::Depth] = depth;
    }
        [[fallthrough]];
    case ProVer_2:
        [[fallthrough]];
    case ProVer_1:;
    }
    //    stream >> *static_cast<AbstractFile*>(this);
    // _read(stream);
}

void File::createGi()
{
    switch (m_gcp.gcType) {
    case Profile:
    case Thermal:
        createGiProfile();
        break;
    case Raster: {
        createGiRaster();
    } break;
    case Voronoi:
        if (m_toolPathss.size() > 1) {
            GraphicsItem* item;
            item = new PathItem(m_toolPathss.last().last(), this);
            item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColor(&GlobalSettings::guiColor(Colors::CutArea));
            itemGroup()->append(item);
            createGiPocket();
        } else
            createGiProfile();
        break;
    case Pocket:
        createGiPocket();
        break;
    case Drill:
        createGiDrill();
        break;
    case LaserHLDI:
        createGiLaser();
        break;
    default:
        break;
    }
}
}
