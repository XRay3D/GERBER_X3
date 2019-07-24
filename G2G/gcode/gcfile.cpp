#include "gcfile.h"

#include "forms/gcodepropertiesform.h"
#include <QFile>
#include <QPainter>
#include <QTextStream>
#include <gi/itemgroup.h>
#include <graphicsview.h>
#include <point.h>
#include <project.h>
#include <settings.h>

namespace GCode {

QString File::lastDir;
/*
  G0X0Y0S1500M03
  M05
  M05 - выкл лазера GRBL
  M03 - вкл лазера GRBL









 */

///////////////////////////////////////////////
void performance(QVector<QPair<cInt, cInt>>& range, Pathss& pathss, const Paths& paths, bool fl = true)
{
    static cInt top = 0;
    static cInt bottom = 0;
    Clipper clipper;
    clipper.AddPaths(paths, ptSubject, true);
    const IntRect rect(clipper.GetBounds());

    if (fl) {
        top = rect.top - 1;
        bottom = rect.bottom + 1;
    }

    const cInt k = rect.right - rect.left;
    Paths paths1;
    Paths paths2;
    qDebug() << ((rect.bottom - rect.top) * dScale);
    if (k < (uScale * 10)) {
        range.append(qMakePair(rect.left, rect.right));
        pathss.append(paths);
    } else {
        cInt c = static_cast<cInt>(k * 0.5);
        Path outerLeft{
            IntPoint(rect.left - 1, top),
            IntPoint(rect.left + c + 1, top),
            IntPoint(rect.left + c + 1, bottom),
            IntPoint(rect.left - 1, bottom)
        };
        clipper.Clear();
        clipper.AddPaths(paths, ptSubject, false);
        clipper.AddPath(outerLeft, ptClip, true);
        clipper.Execute(ctIntersection, paths1, pftPositive);
        performance(range, pathss, paths1, false);

        Path outerRight{
            IntPoint(rect.right - c - 1, top),
            IntPoint(rect.right + 1, top),
            IntPoint(rect.right + 1, bottom),
            IntPoint(rect.right - c - 1, bottom)
        };
        clipper.Clear();
        clipper.AddPaths(paths, ptSubject, false);
        clipper.AddPath(outerRight, ptClip, true);
        clipper.Execute(ctIntersection, paths2, pftPositive);
        performance(range, pathss, paths2, false);
    }
}

File::File(const Paths& toolPaths, const Tool& tool, double depth, GCodeType type, const Paths& pocketPaths)
    : m_pocketPaths(pocketPaths)
    , m_type(type)
    , m_toolPaths(toolPaths)
    , m_tool(tool)
    , m_depth(depth)
{
    createGi();
}

Paths File::getPaths() const
{
    return m_toolPaths;
}

void File::write(const QString& name)
{
    setLastDir(name);
    if (!name.isEmpty())
        m_name = name;
    switch (m_type) {
    case Profile:
    case Pocket:
    case Voronoi:
    case Thermal:
        saveProfilePocket();
        break;
    case Drill:
        saveDrill();
        break;
    default:
        break;
    }
}

void File::saveDrill()
{
    statFile();

    QPolygonF path(toQPolygon(m_toolPaths.first()));

    const double k = Pin::min() + Pin::max();

    if (m_side) {
        for (QPointF& point : path) {
            point.rx() = -point.x() + k;
        }
    }

    for (QPointF& point : path)
        point -= GCodePropertiesForm::zeroPoint->pos();

    for (QPointF& point : path) {
        qDebug() << "saveDrill" << point << path.size() << m_tool.getDepth();
        startPath(point);
        const double depth = m_depth;
        for (int i = 1; depth > m_tool.passDepth() * i; ++i) {
            sl.append(formated({ g1(), z(-m_tool.passDepth() * i), feed(m_tool.plungeRate()) }));
            sl.append(formated({ g0(), z(0.0) }));
        }
        sl.append(formated({ g1(), z(-(depth + m_tool.getDepth())), feed(m_tool.plungeRate()) }));
        endPath();
    }
    endFile();
}

void File::saveProfilePocket()
{
    statFile();

    QVector<QPolygonF> paths(toQPolygons(m_toolPaths));

    const double k = Pin::min() + Pin::max();

    if (m_side) {
        for (QPolygonF& path : paths) {
            std::reverse(path.begin(), path.end());
            for (QPointF& point : path) {
                point.rx() = -point.x() + k;
            }
        }
    }
    for (QPolygonF& path : paths) {
        for (QPointF& point : path) {
            point -= GCodePropertiesForm::zeroPoint->pos();
        }
    }

    for (int i = 1; m_depth > m_tool.passDepth() * i; ++i) {
        for (const QPolygonF& path : paths) {
            startPath(path.first());
            sl.append(formated({ g1(), z(-m_tool.passDepth() * i), feed(m_tool.plungeRate()) })); //start z
            bool skip = true;
            for (const QPointF& point : path) {
                if (skip)
                    skip = false;
                else
                    sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()) }));
            }
            endPath();
        }
    }

    for (const QPolygonF& path : paths) {
        startPath(path.first());
        sl.append(formated({ g1(), z(-m_depth), feed(m_tool.plungeRate()) })); //start z
        bool skip = true;
        for (const QPointF& point : path) {
            if (skip)
                skip = false;
            else
                sl.append(formated({ g1(), x(point.x()), y(point.y()), feed(m_tool.feedRate()) }));
        }
        endPath();
    }

    endFile();
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
    sl.append(formated({ g0(), x(point.x()), y(point.y()), s(m_tool.spindleSpeed()) })); //start xy
    sl.append(formated({ g0(), z(GCodePropertiesForm::plunge) })); //start z
    lastValues[AlwaysF].clear();
}

void File::endPath()
{
    sl.append(formated({ g0(), z(GCodePropertiesForm::clearence) }));
}

void File::statFile()
{

    const QList<QChar> cl{ 'G', 'X', 'Y', 'Z', 'F', 'S' };
    for (bool& fl : FormatFlags) {
        fl = false;
    }
    const QString formaZ(Settings::gCodeFormat());
    for (int i = 0; i < cl.size(); ++i) {
        const int index = formaZ.indexOf(cl[i], 0, Qt::CaseInsensitive);
        if (index != -1) {
            FormatFlags[i + AlwaysG] = formaZ[index + 1] == '+';
            FormatFlags[i + SpaceG] = formaZ[index + 2] == ' ';
        }
    }

    for (QString& str : lastValues)
        str.clear();
    sl.clear();
    sl.append(Settings::startGCode()); //"G21 G17 G90"); //G17 XY plane

    //    QPointF home(MaterialSetup::homePos - MaterialSetup::zeroPos);
    //    sl.append(g0() + x(home.x()) + y(home.y()) + s(m_tool.spindleSpeed) + "M3"); //HomeXY
    sl.append(formated({ g0(), z(GCodePropertiesForm::safeZ) })); //HomeZ
    //    sl.append(s(m_tool.spindleSpeed) + " M3"); //HomeXY
}

void File::endFile()
{
    sl.append(formated({ g0(), z(GCodePropertiesForm::safeZ) })); //HomeZ
    QPointF home(GCodePropertiesForm::homePoint->pos() - GCodePropertiesForm::zeroPoint->pos());
    sl.append(formated({ g0(), x(home.x()), y(home.y()) })); //HomeXY
    //    sl.append("M5"); //HomeXY
    sl.append(Settings::endGCode());

    QFile file(m_name);
    //    QString str(m_fileName);
    //    QFile file(str.insert(str.length() - 4, QString("(Top)|(Bot)").split('|')[side()]));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (QString& s : sl)
            if (!s.isEmpty())
                out << s << endl;
    }
    file.close();
}

QString File::formated(const QList<QString> data)
{
    static const QList<QChar> cl{ 'G', 'X', 'Y', 'Z', 'F', 'S' };
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

void File::write(QDataStream& stream) const
{
    stream << m_pocketPaths;
    stream << m_type;
    stream << m_toolPaths;
    stream << m_tool;
    stream << m_depth;
    _write(stream);
}

void File::read(QDataStream& stream)
{
    stream >> m_pocketPaths;
    stream >> (int&)(m_type);
    stream >> m_toolPaths;
    stream >> m_tool;
    stream >> m_depth;
    _read(stream);
}

void File::createGi()
{
    switch (Project::ver()) {
    case G2G_Ver_1:
        for (Path& p1 : m_pocketPaths) {
            for (IntPoint& p : p1) {
                p.X *= 0.01;
                p.Y *= 0.01;
            }
        }
        for (Path& p1 : m_toolPaths) {
            for (IntPoint& p : p1) {
                p.X *= 0.01;
                p.Y *= 0.01;
            }
        }
        for (Path& p1 : m_g0path) {
            for (IntPoint& p : p1) {
                p.X *= 0.01;
                p.Y *= 0.01;
            }
        }
        break;
    default:
        break;
    }

    GraphicsItem* item;
    switch (m_type) {
    case Profile:
    case Voronoi:
    case Thermal:
        for (const Path& path : m_toolPaths) {
            //qDebug() << path;
            item = new PathItem(path, this);
            item->setPen(QPen(Qt::black, m_tool.getDiameter(m_depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColor(Settings::color(Colors::CutArea));
            itemGroup()->append(item);
        }
        m_g0path.reserve(m_toolPaths.size() * 2);
        for (int i = 0; i < m_toolPaths.size(); ++i) {
            item = new PathItem(m_toolPaths[i], this);
#ifdef QT_DEBUG
            int k = (m_toolPaths.size() > 1) ? (300.0 / (m_toolPaths.size() - 1)) * i : 0;
            QColor* c = new QColor;
            *c = QColor::fromHsv(k, 255, 255, 255);
            item->setPenColor(*c);
#else
            item->setPenColor(Settings::color(Colors::ToolPath));
#endif
            itemGroup()->append(item);
            if (i < m_toolPaths.size() - 1) {
                m_g0path.append({ m_toolPaths[i].last(), m_toolPaths[i + 1].first() });
            }
        }

        //        for (const Path& path : tmpPaths2) {
        //            item = new PathItem({ path } , this);
        //            item->setPenColor(Settings::color(Colors::ToolPath));
        //            itemGroup()->append(item);
        //            g0path.append(path.first());
        //        }

        item = new PathItem(m_g0path);
        item->setPenColor(Settings::color(Colors::G0));
        itemGroup()->append(item);
        break;
    case Pocket:
        if (0) {
            //fast render
            //            Paths tmpPaths;
            //            Pathss pathss;
            //            Clipper clipper;
            //            QVector<QPair<cInt, cInt>> range;
            //            item = new GerberItem(m_pocketPaths, nullptr);
            //            item->setPen(QPen(Qt::black, tool.getDiameter(depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            //            item->setPenColor(Settings::color(Colors::CutArea));
            //            item->setBrushColor(Settings::color(Colors::CutArea));
            //            item->setAcceptHoverEvents(false);
            //            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            //            g0path.reserve(toolPaths.size());
            //            itemGroup()->append(item);
            //            clipper.Clear();
            //            clipper.AddPaths(tmpPaths2, ptSubject, true);
            //            const IntRect rect(clipper.GetBounds());
            //            for (Path& path : tmpPaths2)
            //                if (path.first() != path.last())
            //                    path.append(path.first());

            //            cInt ky = uScale * 10;
            //            performance(range, pathss, tmpPaths2);
            //            qDebug() << range.size() << pathss.size();
            //            for (int i = 0; i < range.size(); ++i) {
            //                for (cInt y = rect.top; y < rect.bottom; y += ky) {
            //                    qDebug() << i << y;
            //                    Path outer{
            //                        IntPoint(range[i].first - 1, y - 1),
            //                        IntPoint(range[i].second + 1, y - 1),
            //                        IntPoint(range[i].second + 1, y + ky + 1),
            //                        IntPoint(range[i].first - 1, y + ky + 1)
            //                    };
            //                    clipper.Clear();
            //                    clipper.AddPaths(pathss[i], ptSubject, false);
            //                    clipper.AddPath(outer, ptClip, true);
            //                    clipper.Execute(ctIntersection, tmpPaths, /*pftNonZero*/ pftPositive);
            //                    item = new PathItem(tmpPaths , this);

            //                    item->setPenColor(Settings::color(Colors::ToolPath));
            //                    item->setAcceptDrops(false);
            //                    item->setAcceptedMouseButtons(Qt::NoButton);
            //                    item->setAcceptHoverEvents(false);
            //                    item->setAcceptTouchEvents(false);
            //                    //item->setActive(false);\ g
            //                    itemGroup()->append(item);
            //                }
            //            }

            //            for (const Path& path : toolPaths)
            //                g0path.append(path.first());
            //            item = new PathItem({ g0path } , this);
            //            item->setPenColor(Settings::color(Colors::G0));
            //            itemGroup()->append(item);
        } else {
            item = new GerberItem(m_pocketPaths, nullptr);
            item->setPen(QPen(Qt::black, m_tool.getDiameter(m_depth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColor(Settings::color(Colors::CutArea));
            item->setBrushColor(Settings::color(Colors::CutArea));
            item->setAcceptHoverEvents(false);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            itemGroup()->append(item);
            m_g0path.reserve(m_toolPaths.size());
            int i = 0;
            for (const Path& path : m_toolPaths) {
                item = new PathItem(path, this);
                item->setPenColor(Settings::color(Colors::ToolPath));
                itemGroup()->append(item);
                if (i < m_toolPaths.size() - 1) {
                    m_g0path.append({ m_toolPaths[i].last(), m_toolPaths[++i].first() });
                }
            }
            item = new PathItem(m_g0path);
            item->setPenColor(Settings::color(Colors::G0));
            itemGroup()->append(item);
        }
        break;
    case Drill:
        for (const IntPoint& point : m_toolPaths.first()) {
            item = new DrillItem(m_tool.diameter(), this);
            item->setPos(toQPointF(point));
            item->setPenColor(Settings::color(Colors::ToolPath));
            item->setBrushColor(Settings::color(Colors::CutArea));
            itemGroup()->append(item);
        }
        item = new PathItem(m_toolPaths);
        item->setPenColor(Settings::color(Colors::G0));
        itemGroup()->append(item);
        break;
    default:
        break;
    }
}
}
