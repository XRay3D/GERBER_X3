#include "gcfile.h"

#include "forms/gcodepropertiesform.h"

#include <QDir>
#include <QFile>
#include <QPainter>
#include <QTextStream>
#include <gi/itemgroup.h>
#include <graphicsview.h>
#include <point.h>
#include <project.h>
#include <settings.h>

namespace GCode {

const QVector<QChar> File::cmdList { 'G', 'X', 'Y', 'Z', 'F', 'S' };
QString File::lastDir;
/*
  G0X0Y0S1500M03
  M05
  M05 - выкл лазера GRBL
  M03 - вкл лазера GRBL
 */

File::File(const Pathss& toolPathss, const GCodeParams& gcp, const Paths& pocketPaths)
    : m_pocketPaths(pocketPaths)
    , m_toolPathss(toolPathss)
    , m_gcp(gcp)
{
    createGi();
}

void File::save(const QString& name)
{
    if (name.isEmpty())
        return;
    setLastDir(name);
    m_name = name;

    initSave();
    addInfo();
    statFile();
    genGcodeAndTile();
    endFile();
    QFile file(m_name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString str;
        for (QString& s : m_gCodeText) {
            if (!s.isEmpty())
                str.append(s);
            if (!str.endsWith('\n'))
                str.append("\n");
        }
        out << str;
    }
    file.close();
}

void File::saveDrill(const QPointF& offset)
{

    QPolygonF path(toQPolygon(m_toolPathss.first().first()));
    path.translate(offset);

    if (m_side == Bottom) {
        const double k = Pin::minX() + Pin::maxX();
        for (QPointF& point : path) {
            point.rx() = -point.x() + k;
        }
    }

    for (QPointF& point : path)
        point -= Marker::get(Marker::Zero)->pos();

    const QVector<double> depths(getDepths());

    for (QPointF& point : path) {
        startPath(point);
        for (int i = 0;;) {
            m_gCodeText.append(formated({ g1(), z(depths[i]), feed(plungeRate) }));
            if (++i == depths.count())
                break;
            m_gCodeText.append(formated({ g0(), z(0.0) }));
        }
        endPath();
    }
}

void File::savePocket(const QPointF& offset)
{
    if (toolType == Tool::Laser) {
        m_gCodeText.append(GlobalSettings::gcSpindleOn());
    } else {
        m_gCodeText.append(GlobalSettings::gcLaserDynamOn());
    }

    QVector<QVector<QPolygonF>> pathss(pss(offset));
    const QVector<double> depths(getDepths());
    for (QVector<QPolygonF>& paths : pathss) {
        startPath(paths.first().first());
        if (toolType == Tool::Laser) {
            bool skip = true;
            for (QPolygonF& path : paths) {
                for (QPointF& point : path) {
                    if (skip)
                        skip = false;
                    else
                        m_gCodeText.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate) }));
                }
            }
        } else {
            for (int i = 0; i < depths.count(); ++i) {
                m_gCodeText.append(formated({ g1(), z(depths[i]), feed(plungeRate) }));
                bool skip = true;
                for (QPolygonF& path : paths) {
                    for (QPointF& point : path) {
                        if (skip)
                            skip = false;
                        else
                            m_gCodeText.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate) }));
                    }
                }
                for (int j = paths.size() - 2; j >= 0 && i < depths.count() - 1; --j) {
                    QPointF& point = paths[j].last();
                    m_gCodeText.append(formated({ g0(), x(point.x()), y(point.y()) }));
                }
                if (paths.size() > 1 && i < (depths.count() - 1))
                    m_gCodeText.append(formated({ g0(), x(paths.first().first().x()), y(paths.first().first().y()) }));
            }
        }
        endPath();
    }
}

void File::saveProfile(const QPointF& offset)
{
    if (toolType == Tool::Laser)
        m_gCodeText.append(GlobalSettings::gcSpindleOn());
    else
        m_gCodeText.append(GlobalSettings::gcLaserDynamOn());

    QVector<QVector<QPolygonF>> pathss(pss(offset));
    const QVector<double> depths(getDepths());

    if (toolType == Tool::Laser) {
        for (QVector<QPolygonF>& paths : pathss) {
            for (QPolygonF& path : paths) {
                startPath(path.first());
                bool skip = true;
                for (QPointF& point : path) {
                    if (skip)
                        skip = false;
                    else
                        m_gCodeText.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate) }));
                }
                endPath();
            }
        }
    } else {
        if (m_gcp.gcType != Raster) {
            for (QVector<QPolygonF>& paths : pathss) {
                for (int i = 0; i < depths.count(); ++i) {
                    for (int j = 0; j < paths.size(); ++j) {
                        QPolygonF& path = paths[j];
                        if (path.first() == path.last()) {
                            startPath(path.first());
                            for (int i = 0; i < depths.count(); ++i) {
                                m_gCodeText.append(formated({ g1(), z(depths[i]), feed(plungeRate) }));
                                bool skip = true;
                                for (QPointF& point : path) {
                                    if (skip)
                                        skip = false;
                                    else
                                        m_gCodeText.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate) }));
                                }
                            }
                            endPath();
                            paths.remove(j--);
                            continue;
                        } else {
                            startPath(path.first());
                            m_gCodeText.append(formated({ g1(), z(depths[i]), feed(plungeRate) }));
                            bool skip = true;
                            for (QPointF& point : path) {
                                if (skip)
                                    skip = false;
                                else
                                    m_gCodeText.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate) }));
                            }
                            endPath();
                        }
                    }
                }
            }
        } else {
            for (QVector<QPolygonF>& paths : pathss) {
                for (int i = 0; i < depths.count(); ++i) {
                    for (QPolygonF& path : paths) {
                        startPath(path.first());
                        m_gCodeText.append(formated({ g1(), z(depths[i]), feed(plungeRate) }));
                        bool skip = true;
                        for (QPointF& point : path) {
                            if (skip)
                                skip = false;
                            else
                                m_gCodeText.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate) }));
                        }
                        endPath();
                    }
                }
            }
        }
    }
}

void File::saveLaser(const QPointF& offset)
{
    if (toolType == Tool::Laser)
        m_gCodeText.append(GlobalSettings::gcSpindleOn());
    else
        m_gCodeText.append(GlobalSettings::gcLaserConstOn());

    QVector<QVector<QPolygonF>> pathss(pss(offset));
    int i = 0;
    m_gCodeText.append(formated({ g0(), x(pathss.first().first().first().x()), y(pathss.first().first().first().y()), z(0.0) }));
    for (QPolygonF& path : pathss.first()) {
        if (i++ % 2) {
            bool skip = true;
            for (QPointF& point : path) {
                if (skip)
                    skip = false;
                else
                    m_gCodeText.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate), speed(spindleSpeed) }));
            }
        } else {
            bool skip = true;
            for (QPointF& point : path) {
                if (skip)
                    skip = false;
                else
                    m_gCodeText.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate), speed(static_cast<int>(0)) }));
            }
        }
    }
    m_gCodeText.append(GlobalSettings::gcLaserDynamOn());
    if (pathss.size() > 1) {
        for (QPolygonF& path : pathss.last()) {
            startPath(path.first());
            bool skip = true;
            for (QPointF& point : path) {
                if (skip)
                    skip = false;
                else
                    m_gCodeText.append(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate) }));
            }
            endPath();
        }
    }
}

GCodeParams File::gcp() const
{
    return m_gcp;
}

QVector<QVector<QPolygonF>> File::pss(const QPointF& offset)
{
    QVector<QVector<QPolygonF>> pathss;
    pathss.reserve(m_toolPathss.size());
    for (const Paths& paths : m_toolPathss) {
        pathss.append(toQPolygons(paths));
    }

    for (QVector<QPolygonF>& paths : pathss)
        for (QPolygonF& path : paths)
            path.translate(offset);

    if (m_side == Bottom) {
        const double k = Pin::minX() + Pin::maxX();
        for (QVector<QPolygonF>& paths : pathss) {
            for (QPolygonF& path : paths) {
                if (toolType != Tool::Laser)
                    std::reverse(path.begin(), path.end());
                for (QPointF& point : path) {
                    point.rx() = -point.x() + k;
                }
            }
        }
    }
    for (QVector<QPolygonF>& paths : pathss) {
        for (QPolygonF& path : paths) {
            for (QPointF& point : path) {
                point -= Marker::get(Marker::Zero)->pos();
            }
        }
    }

    return pathss;
}

QVector<QPolygonF> File::ps(const QPointF& offset)
{
    QVector<QPolygonF> paths(toQPolygons(m_toolPathss.first()));

    for (QPolygonF& path : paths)
        path.translate(offset);

    if (m_side == Bottom) {
        const double k = Pin::minX() + Pin::maxX();
        for (QPolygonF& path : paths) {
            if (toolType != Tool::Laser)
                std::reverse(path.begin(), path.end());
            for (QPointF& point : path) {
                point.rx() = -point.x() + k;
            }
        }
    }
    for (QPolygonF& path : paths) {
        for (QPointF& point : path) {
            point -= Marker::get(Marker::Zero)->pos();
        }
    }

    return paths;
}

QVector<double> File::getDepths()
{
    const auto gDepth { m_gcp.getDepth() };
    if (gDepth < m_gcp.getTool().passDepth() || qFuzzyCompare(gDepth, m_gcp.getTool().passDepth()))
        return { -gDepth - m_gcp.getTool().getDepth() };

    const int count = static_cast<int>(ceil(gDepth / m_gcp.getTool().passDepth()));
    const double depth = gDepth / count;
    QVector<double> depths(count);
    for (int i = 0; i < count; ++i)
        depths[i] = (i + 1) * -depth;
    depths.last() = -gDepth - m_gcp.getTool().getDepth();
    return depths;
}

void File::initSave()
{
    m_gCodeText.clear();

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

    *const_cast<double*>(&feedRate) = m_gcp.getTool().feedRate();
    *const_cast<double*>(&plungeRate) = m_gcp.getTool().plungeRate();
    *const_cast<int*>(&spindleSpeed) = m_gcp.getTool().spindleSpeed();
    *const_cast<int*>(&toolType) = m_gcp.getTool().type();
}

void File::genGcodeAndTile()
{
    const QRectF rect = App::project()->worckRect();
    for (int x = 0; x < App::project()->stepsX(); ++x) {
        for (int y = 0; y < App::project()->stepsY(); ++y) {
            const QPointF offset(
                (rect.width() + App::project()->spaceX()) * x,
                (rect.height() + App::project()->spaceY()) * y);
            switch (m_gcp.gcType) {
            case Pocket:
                savePocket(offset);
                break;
            case Voronoi:
                if (m_toolPathss.size() > 1)
                    savePocket(offset);
                else
                    saveProfile(offset);
                break;
            case Profile:
            case Thermal:
            case Raster:
                saveProfile(offset);
                break;
            case Drill:
                saveDrill(offset);
                break;
            case LaserHLDI:
                saveLaser(offset);
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
        m_gCodeText.append(QString(";\tName:\t%1").arg(shortName()));
        m_gCodeText.append(QString(";\tTool:\t%1").arg(m_gcp.getTool().name()));
        m_gCodeText.append(QString(";\tDepth:\t%1").arg(m_gcp.getDepth()));
        m_gCodeText.append(QString(";\tSide:\t%1").arg(QStringList { "Top", "Bottom" }[side()]));
    }
}

GCodeType File::gtype() const
{
    return m_gcp.gcType;
}

QString File::getLastDir()
{
    if (GlobalSettings::gcSameFolder())
        lastDir = QFileInfo(App::project()->name()).absolutePath();
    else if (lastDir.isEmpty()) {
        QSettings settings;
        lastDir = settings.value("LastGCodeDir").toString();
        if (lastDir.isEmpty())
            lastDir = QFileInfo(App::project()->name()).absolutePath();
        settings.setValue("LastGCodeDir", lastDir);
    }
    qDebug() << QFileInfo(lastDir).absolutePath() << lastDir;
    return lastDir += '/';
}

void File::setLastDir(QString value)
{
    if (GlobalSettings::gcSameFolder())
        return;
    value = QFileInfo(value).absolutePath(); //value.left(value.lastIndexOf('/') + 1);
    if (lastDir != value) {
        lastDir = value;
        QSettings settings;
        settings.setValue("LastGCodeDir", lastDir);
    }
}

void File::startPath(const QPointF& point)
{
    if (toolType == Tool::Laser) {
        m_gCodeText.append(formated({ g0(), x(point.x()), y(point.y()), speed(0) })); //start xy
        m_gCodeText.append(formated({ g1(), speed(spindleSpeed) }));
    } else {
        m_gCodeText.append(formated({ g0(), x(point.x()), y(point.y()), speed(spindleSpeed) })); //start xy
        m_gCodeText.append(formated({ g0(), z(GCodePropertiesForm::plunge) })); //start z
        lastValues[AlwaysF].clear();
    }
}

void File::endPath()
{
    if (toolType == Tool::Laser) {
    } else {
        m_gCodeText.append(formated({ g0(), z(GCodePropertiesForm::clearence) }));
    }
}

void File::statFile()
{
    QString str(GlobalSettings::gcStart()); //"G21 G17 G90"); //G17 XY plane
    str.replace(QRegExp("S\\?"), formated({ speed(spindleSpeed) }));
    m_gCodeText.append(str);
    if (toolType != Tool::Laser) {
        m_gCodeText.append(formated({ g0(), z(GCodePropertiesForm::safeZ) })); //HomeZ
    }
}

void File::endFile()
{
    if (toolType != Tool::Laser) {
        m_gCodeText.append(formated({ g0(), z(GCodePropertiesForm::safeZ) })); //HomeZ
    }
    QPointF home(Marker::get(Marker::Home)->pos() - Marker::get(Marker::Zero)->pos());
    m_gCodeText.append(formated({ g0(), x(home.x()), y(home.y()) })); //HomeXY
    m_gCodeText.append(GlobalSettings::gcEnd());
}

QString File::formated(const QList<QString>& data)
{
    QString ret;
    for (const QString& str : data) {
        const int index = cmdList.indexOf(str.front().toUpper());
        if (index != -1) {
            if (formatFlags[AlwaysG + index] || lastValues[index] != str) {
                lastValues[index] = str;
                ret += str + (formatFlags[SpaceG + index] ? " " : "");
            }
        }
    }
    return ret.trimmed();
}

QList<QString> File::gCodeText() const { return m_gCodeText; }

void File::createGiDrill()
{
    GraphicsItem* item;
    for (const IntPoint& point : m_toolPathss.first().first()) {
        item = new DrillItem(m_gcp.getTool().diameter(), this);
        item->setPos(toQPointF(point));
        item->setPenColor(GlobalSettings::guiColor(Colors::ToolPath));
        item->setBrushColor(GlobalSettings::guiColor(Colors::CutArea));
        itemGroup()->append(item);
    }
    item = new PathItem(m_toolPathss.first().first());
    item->setPenColor(GlobalSettings::guiColor(Colors::G0));
    itemGroup()->append(item);
}

void File::createGiPocket()
{
    GraphicsItem* item;
    if (m_pocketPaths.size()) {
        item = new GerberItem(m_pocketPaths, nullptr);
        item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColor(GlobalSettings::guiColor(Colors::CutArea));
        item->setBrushColor(GlobalSettings::guiColor(Colors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->append(item);
    }
    m_g0path.reserve(m_toolPathss.size());
    int i = 0;
    for (const Paths& paths : m_toolPathss) {
        int k = static_cast<int>((m_toolPathss.size() > 1) ? (300.0 / (m_toolPathss.size() - 1)) * i : 0);
        QColor* c = new QColor;
        *c = QColor::fromHsv(k, 255, 255, 255);

        for (const Path& path : paths) {
            item = new PathItem(path, this);
#ifdef QT_DEBUG
            item->setPenColor(*c);
#else
            item->setPenColor(GlobalSettings::guiColor(Colors::ToolPath));
#endif
            itemGroup()->append(item);
        }

        {
            Paths g1path;
            for (int i = 0; i < paths.count() - 1; ++i)
                g1path.append({ paths[i].last(), paths[i + 1].first() });
            item = new PathItem(g1path);
#ifdef QT_DEBUG
            item->setPenColor(*new QColor(0, 0, 255));
#else
            item->setPenColor(GlobalSettings::guiColor(Colors::ToolPath));
#endif
            itemGroup()->append(item);
        }

        if (i < m_toolPathss.size() - 1) {
            m_g0path.append({ m_toolPathss[i].last().last(), m_toolPathss[++i].first().first() });
        }
    }
    item = new PathItem(m_g0path);
    item->setPenColor(GlobalSettings::guiColor(Colors::G0));
    itemGroup()->append(item);
}

void File::createGiProfile()
{
    GraphicsItem* item;
    for (const Paths& paths : m_toolPathss) {
        item = new PathItem(paths, this);
        item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColor(GlobalSettings::guiColor(Colors::CutArea));
        itemGroup()->append(item);
    }
    int i = 0;
    for (const Paths& paths : m_toolPathss) {
        item = new PathItem(m_toolPathss[i], this);
        item->setPenColor(GlobalSettings::guiColor(Colors::ToolPath));
        itemGroup()->append(item);
        for (int i = 0; i < paths.count() - 1; ++i)
            m_g0path.append({ paths[i].last(), paths[i + 1].first() });
        if (i < m_toolPathss.size() - 1) {
            m_g0path.append({ m_toolPathss[i].last().last(), m_toolPathss[++i].first().first() });
        }
    }

    item = new PathItem(m_g0path);
    //    item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColor(GlobalSettings::guiColor(Colors::G0));
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
        item = new GerberItem(m_pocketPaths, nullptr);
        item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColor(GlobalSettings::guiColor(Colors::CutArea));
        item->setBrushColor(GlobalSettings::guiColor(Colors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->append(item);
    } else {
        for (const Paths& paths : m_toolPathss) {
            item = new PathItem(paths, this);
            item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColor(GlobalSettings::guiColor(Colors::CutArea));
            itemGroup()->append(item);
        }
    }
    int i = 0;
    for (const Paths& paths : m_toolPathss) {
        item = new PathItem(paths, this);
        item->setPenColor(GlobalSettings::guiColor(Colors::ToolPath));
        itemGroup()->append(item);
        for (int i = 0; i < paths.count() - 1; ++i)
            m_g0path.append({ paths[i].last(), paths[i + 1].first() });
        if (i < m_toolPathss.size() - 1) {
            m_g0path.append({ m_toolPathss[i].last().last(), m_toolPathss[++i].first().first() });
        }
    }
    item = new PathItem(m_g0path);
    item->setPenColor(GlobalSettings::guiColor(Colors::G0));
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
    item->setPenColor(GlobalSettings::guiColor(Colors::CutArea));
    itemGroup()->append(item);

    item = new PathItem(paths, this);
    item->setPenColor(GlobalSettings::guiColor(Colors::ToolPath));
    itemGroup()->append(item);

    item = new PathItem(m_g0path);
    item->setPenColor(GlobalSettings::guiColor(Colors::G0));
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
    //    case G2G_Ver_1:;
    //    }
    _write(stream);
}

void File::read(QDataStream& stream)
{
    auto& gcp = *const_cast<GCodeParams*>(&m_gcp);
    switch (App::project()->ver()) {
    case G2G_Ver_4:
        stream >> gcp;
        stream >> m_pocketPaths;
        stream >> m_toolPathss;
        break;
    case G2G_Ver_3: {
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
    case G2G_Ver_2:
        [[fallthrough]];
    case G2G_Ver_1:;
    }
    _read(stream);
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
            item->setPenColor(GlobalSettings::guiColor(Colors::CutArea));
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
