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

const QList<QChar> File::cl { 'G', 'X', 'Y', 'Z', 'F', 'S' };
QString File::lastDir;
/*
  G0X0Y0S1500M03
  M05
  M05 - выкл лазера GRBL
  M03 - вкл лазера GRBL
 */

File::File(const Pathss& toolPathss, const Tool& tool, double depth, GCodeType type, const Paths& pocketPaths)
    : m_pocketPaths(pocketPaths)
    , m_type(type)
    , m_toolPathss(toolPathss)
    , m_tool(tool)
    , m_depth(depth)
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
    genGcode();
    endFile();
    QFile file(m_name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString str;
        for (QString& s : sl) {
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

    if (m_side) {
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
            sl.append(formated({ g1(), z(depths[i]), feed(m_tool.plungeRate()) }));
            if (++i == depths.count())
                break;
            sl.append(formated({ g0(), z(0.0) }));
        }
        endPath();
    }
}

void File::savePocket(const QPointF& offset)
{
    QVector<QVector<QPolygonF>> pathss(pss(offset));
    const QVector<double> depths(getDepths());
    for (QVector<QPolygonF>& paths : pathss) {
        startPath(paths.first().first());
        if (m_tool.type() == Tool::Laser) {
            bool skip = true;
            for (QPolygonF& path : paths) {
                for (QPointF& point : path) {
                    if (skip)
                        skip = false;
                    else
                        sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()) }));
                }
            }
        } else {
            for (int i = 0; i < depths.count(); ++i) {
                sl.append(formated({ g1(), z(depths[i]), feed(m_tool.plungeRate()) }));
                bool skip = true;
                for (QPolygonF& path : paths) {
                    for (QPointF& point : path) {
                        if (skip)
                            skip = false;
                        else
                            sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()) }));
                    }
                }
                for (int j = paths.size() - 2; j >= 0 && i < depths.count() - 1; --j) {
                    QPointF& point = paths[j].last();
                    sl.append(formated({ g0(), x(point.x()), y(point.y()) }));
                }
                if (paths.size() > 1 && i < (depths.count() - 1))
                    sl.append(formated({ g0(), x(paths.first().first().x()), y(paths.first().first().y()) }));
            }
        }
        endPath();
    }
}

void File::saveProfile(const QPointF& offset)
{
    QVector<QVector<QPolygonF>> pathss(pss(offset));
    const QVector<double> depths(getDepths());

    if (m_tool.type() == Tool::Laser) {
        for (QVector<QPolygonF>& paths : pathss) {
            for (QPolygonF& path : paths) {
                startPath(path.first());
                bool skip = true;
                for (QPointF& point : path) {
                    if (skip)
                        skip = false;
                    else
                        sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()) }));
                }
                endPath();
            }
        }
    } else {
        if (m_type != Raster) {
            for (QVector<QPolygonF>& paths : pathss) {
                for (int i = 0; i < depths.count(); ++i) {
                    for (int j = 0; j < paths.size(); ++j) {
                        QPolygonF& path = paths[j];
                        if (path.first() == path.last()) {
                            startPath(path.first());
                            for (int i = 0; i < depths.count(); ++i) {
                                sl.append(formated({ g1(), z(depths[i]), feed(m_tool.plungeRate()) }));
                                bool skip = true;
                                for (QPointF& point : path) {
                                    if (skip)
                                        skip = false;
                                    else
                                        sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()) }));
                                }
                            }
                            endPath();
                            paths.remove(j--);
                            continue;
                        } else {
                            startPath(path.first());
                            sl.append(formated({ g1(), z(depths[i]), feed(m_tool.plungeRate()) }));
                            bool skip = true;
                            for (QPointF& point : path) {
                                if (skip)
                                    skip = false;
                                else
                                    sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()) }));
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
                        sl.append(formated({ g1(), z(depths[i]), feed(m_tool.plungeRate()) }));
                        bool skip = true;
                        for (QPointF& point : path) {
                            if (skip)
                                skip = false;
                            else
                                sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()) }));
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
    QVector<QVector<QPolygonF>> pathss(pss(offset));
    int i = 0;
    sl.append(formated({ g0(), x(pathss.first().first().first().x()), y(pathss.first().first().first().y()), z(0.0) }));
    for (QPolygonF& path : pathss.first()) {
        if (i++ % 2) { //laser on
            //            sl.append("M03");
            //            FormatFlags[AlwaysS] = true;
            //            sl.append(formated({ g1(), s(static_cast<int>(m_tool.spindleSpeed())) }));
            bool skip = true;
            for (QPointF& point : path) {
                if (skip)
                    skip = false;
                else
                    sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()), s(static_cast<int>(m_tool.spindleSpeed())) }));
            }
        } else { //laser off
            //            sl.append("M05");
            bool skip = true;
            for (QPointF& point : path) {
                if (skip)
                    skip = false;
                else
                    sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()), s(static_cast<int>(0)) }));
            }
        }
    }
    sl.append("M04");
    if (pathss.size() > 1) {
        for (QPolygonF& path : pathss.last()) {
            startPath(path.first());
            bool skip = true;
            for (QPointF& point : path) {
                if (skip)
                    skip = false;
                else
                    sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()) }));
            }
            endPath();
        }
    }
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
                if (m_tool.type() != Tool::Laser)
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
            if (m_tool.type() != Tool::Laser)
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
    if (m_depth < m_tool.passDepth() || qFuzzyCompare(m_depth, m_tool.passDepth()))
        return { -m_depth - m_tool.getDepth() };

    const int count = static_cast<int>(ceil(m_depth / m_tool.passDepth()));
    const double depth = m_depth / count;
    QVector<double> depths(count);
    for (int i = 0; i < count; ++i)
        depths[i] = (i + 1) * -depth;
    depths.last() = -m_depth - m_tool.getDepth();
    return depths;
}

void File::initSave()
{
    sl.clear();
    static const QList<QChar> cl { 'G', 'X', 'Y', 'Z', 'F', 'S' };
    memset(FormatFlags, 0, sizeof(bool) * Size);
    //    for (bool& fl : FormatFlags) {
    //        fl = false;
    //    }
    const QString format(Settings::gCodeFormat());
    for (int i = 0; i < cl.size(); ++i) {
        const int index = format.indexOf(cl[i], 0, Qt::CaseInsensitive);
        if (index != -1) {
            FormatFlags[i + AlwaysG] = format[index + 1] == '+';
            if ((index + 2) < format.size())
                FormatFlags[i + SpaceG] = format[index + 2] == ' ';
        }
    }

    for (QString& str : lastValues)
        str.clear();
}

void File::genGcode()
{
    QRectF rect = Project::instance()->worckRect();
    for (int x = 0; x < Project::instance()->stepsX(); ++x) {
        for (int y = 0; y < Project::instance()->stepsY(); ++y) {
            QPointF offset(
                (rect.width() + Project::instance()->spasingX()) * x,
                (rect.height() + Project::instance()->spasingY()) * y);
            switch (m_type) {
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
        }
    }
}

Tool File::getTool() const
{
    return m_tool;
}

void File::addInfo(bool fl)
{
    if (Settings::gcinfo() || fl) {
        sl.append(QString(";\tName: %1").arg(shortName()));
        sl.append(QString(";\tTool: %1").arg(m_tool.name()));
        sl.append(QString(";\tDepth: %1").arg(m_depth));
        sl.append(QString(";\tSide: %1").arg(QStringList { "Top", "Bottom" }[side()]));
    }
}

GCodeType File::gtype() const
{
    return m_type;
}

QString File::getLastDir()
{
    if (lastDir.isEmpty()) {
        QSettings settings;
        lastDir = settings.value("LastGCodeDir").toString();
        if (lastDir.isEmpty()) {
            lastDir = Project::instance()->name();
            lastDir = lastDir.left(lastDir.lastIndexOf('/') + 1);
        }
        settings.setValue("LastGCodeDir", lastDir);
    }
    return lastDir;
}

void File::setLastDir(QString value)
{
    value = value.left(value.lastIndexOf('/') + 1);
    if (lastDir != value) {
        lastDir = value;
        QSettings settings;
        settings.setValue("LastGCodeDir", lastDir);
    }
}

void File::startPath(const QPointF& point)
{
    if (m_tool.type() == Tool::Laser) {
        sl.append(formated({ g0(), x(point.x()), y(point.y()), s(0) })); //start xy
        sl.append(formated({ g1(), s(static_cast<int>(m_tool.spindleSpeed())) }));
    } else {
        sl.append(formated({ g0(), x(point.x()), y(point.y()), s(static_cast<int>(m_tool.spindleSpeed())) })); //start xy
        sl.append(formated({ g0(), z(GCodePropertiesForm::plunge) })); //start z
        lastValues[AlwaysF].clear();
    }
}

void File::endPath()
{
    if (m_tool.type() == Tool::Laser) {
        //sl.append("M05");
    } else {
        sl.append(formated({ g0(), z(GCodePropertiesForm::clearence) }));
    }
}

void File::statFile()
{

    QString str(Settings::startGCode()); //"G21 G17 G90"); //G17 XY plane
    str.replace(QRegExp("S\\?"), formated({ s(static_cast<int>(m_tool.spindleSpeed())) }));
    sl.append(str);
    if (m_tool.type() != Tool::Laser) {
        sl.append(formated({ g0(), z(GCodePropertiesForm::safeZ) })); //HomeZ
    }
}

void File::endFile()
{
    if (m_tool.type() != Tool::Laser) {
        sl.append(formated({ g0(), z(GCodePropertiesForm::safeZ) })); //HomeZ
    }
    QPointF home(Marker::get(Marker::Home)->pos() - Marker::get(Marker::Zero)->pos());
    sl.append(formated({ g0(), x(home.x()), y(home.y()) })); //HomeXY
    sl.append(Settings::endGCode());
}

QString File::formated(const QList<QString> data)
{
    QString ret;
    for (const QString& str : data) {
        const int index = cl.indexOf(str.front().toUpper());
        if (index != -1) {
            if (lastValues[index] != str || FormatFlags[AlwaysG + index]) {
                lastValues[index] = str;
                ret += str + (FormatFlags[SpaceG + index] ? " " : "");
            }
        }
    }
    return ret.trimmed();
}

QList<QString> File::getSl() const
{
    return sl;
}

void File::createGiDrill()
{
    GraphicsItem* item;
    for (const IntPoint& point : m_toolPathss.first().first()) {
        item = new DrillItem(m_tool.diameter(), this);
        item->setPos(toQPointF(point));
        item->setPenColor(Settings::color(Colors::ToolPath));
        item->setBrushColor(Settings::color(Colors::CutArea));
        itemGroup()->append(item);
    }
    item = new PathItem(m_toolPathss.first().first());
    item->setPenColor(Settings::color(Colors::G0));
    itemGroup()->append(item);
}

void File::createGiPocket()
{
    GraphicsItem* item;
    if (m_pocketPaths.size()) {
        item = new GerberItem(m_pocketPaths, nullptr);
        item->setPen(QPen(Qt::black, m_tool.getDiameter(m_depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColor(Settings::color(Colors::CutArea));
        item->setBrushColor(Settings::color(Colors::CutArea));
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
            item->setPenColor(Settings::color(Colors::ToolPath));
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
            item->setPenColor(Settings::color(Colors::ToolPath));
#endif
            itemGroup()->append(item);
        }

        if (i < m_toolPathss.size() - 1) {
            m_g0path.append({ m_toolPathss[i].last().last(), m_toolPathss[++i].first().first() });
        }
    }
    item = new PathItem(m_g0path);
    item->setPenColor(Settings::color(Colors::G0));
    itemGroup()->append(item);
}

void File::createGiProfile()
{
    GraphicsItem* item;
    for (const Paths& paths : m_toolPathss) {
        item = new PathItem(paths, this);
        item->setPen(QPen(Qt::black, m_tool.getDiameter(m_depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColor(Settings::color(Colors::CutArea));
        itemGroup()->append(item);
    }
    int i = 0;
    for (const Paths& paths : m_toolPathss) {
        item = new PathItem(m_toolPathss[i], this);
        item->setPenColor(Settings::color(Colors::ToolPath));
        itemGroup()->append(item);
        for (int i = 0; i < paths.count() - 1; ++i)
            m_g0path.append({ paths[i].last(), paths[i + 1].first() });
        if (i < m_toolPathss.size() - 1) {
            m_g0path.append({ m_toolPathss[i].last().last(), m_toolPathss[++i].first().first() });
        }
    }

    item = new PathItem(m_g0path);
    //    item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColor(Settings::color(Colors::G0));
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
        item->setPen(QPen(Qt::black, m_tool.getDiameter(m_depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColor(Settings::color(Colors::CutArea));
        item->setBrushColor(Settings::color(Colors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->append(item);
    } else {
        for (const Paths& paths : m_toolPathss) {
            item = new PathItem(paths, this);
            item->setPen(QPen(Qt::black, m_tool.getDiameter(m_depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColor(Settings::color(Colors::CutArea));
            itemGroup()->append(item);
        }
    }
    int i = 0;
    for (const Paths& paths : m_toolPathss) {
        item = new PathItem(paths, this);
        item->setPenColor(Settings::color(Colors::ToolPath));
        itemGroup()->append(item);
        for (int i = 0; i < paths.count() - 1; ++i)
            m_g0path.append({ paths[i].last(), paths[i + 1].first() });
        if (i < m_toolPathss.size() - 1) {
            m_g0path.append({ m_toolPathss[i].last().last(), m_toolPathss[++i].first().first() });
        }
    }
    item = new PathItem(m_g0path);
    item->setPenColor(Settings::color(Colors::G0));
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
    item->setPen(QPen(Qt::black, m_tool.getDiameter(m_depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item->setPenColor(Settings::color(Colors::CutArea));
    itemGroup()->append(item);

    item = new PathItem(paths, this);
    item->setPenColor(Settings::color(Colors::ToolPath));
    itemGroup()->append(item);

    item = new PathItem(m_g0path);
    item->setPenColor(Settings::color(Colors::G0));
    itemGroup()->append(item);
}

void File::write(QDataStream& stream) const
{
    stream << m_pocketPaths;
    stream << m_type;
    stream << m_toolPathss;
    stream << m_tool;
    stream << m_depth;
    _write(stream);
}

void File::read(QDataStream& stream)
{
    stream >> m_pocketPaths;
    stream >> m_type;
    stream >> m_toolPathss;
    stream >> m_tool;
    stream >> m_depth;
    _read(stream);
}

void File::createGi()
{

    switch (m_type) {
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
            item->setPen(QPen(Qt::black, m_tool.getDiameter(m_depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColor(Settings::color(Colors::CutArea));
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
