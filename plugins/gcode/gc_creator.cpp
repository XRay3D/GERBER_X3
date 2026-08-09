/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_creator.h"
// #include "gc_file.h"
#include "gc_file.h"
#include "gc_types.h"

#include "app.h"
#include "gi.h"
#include "gi_datapath.h"
#include "gi_dbg.h"
#include "gi_error.h"
#include "gi_gcpath.h"
#include "gi_point.h"
#include "md5.h"

#include "project.h"
#include "utils.h"
#ifdef Q_OS_UNIX
    #undef emit
    #include <execution>
    #define emit
#endif
#include <forward_list>
#include <numbers>
#include <set>
#include <stdexcept>

class GCDbgFile final : public GCode::File {
    QColor color;

public:
    explicit GCDbgFile(GCode::Params&& gcp, QColor color)
        : GCode::File{std::move(gcp)}
        , color{color} {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
    void write(QDataStream& stream [[maybe_unused]]) const override { }
    void read(QDataStream& stream [[maybe_unused]]) override { }
    void initFrom(AbstractFile* file [[maybe_unused]]) override { qWarning(__FUNCTION__); }
    QIcon icon() const override { return QIcon::fromTheme(u"crosshairs"_s); }
    uint32_t type() const override { return GC_DBG_FILE; }
    void createGi() override {
        Gi::Item* item = new Gi::GcPath{gcp.pocketAreaCurves().contours(), this};
        // item->setPen(QPen(color, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        // item->setPenColorPtr(&color);
        itemGroup()->push_back(item);
        // for(int i{}; i < pocketAreaPaths_.size() - 1; ++i)
        // g0path_.emplace_back(Geo::Polyline{pocketAreaPaths_[i].back(), pocketAreaPaths_[i + 1].front()});
        // item = new Gi::GcPath{g0path_};
        // item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
        // itemGroup()->push_back(item);
        itemGroup()->setVisible(true);
    }
    void genGcodeAndTile() override { } // saveLaserProfile({});
    // AbstractFile interface
};

void dbgPaths(Geo::Polylines ps, const QString& fileName, QColor color, bool close, const Tool& tool) {
    std::erase_if(ps, [](const Geo::Polyline& path) { return path.size() < 2; });
    if(ps.empty())
        return;
    if(close)
        r::for_each(ps, [](Geo::Polyline& p) { p.close(); });
    GCode::Params gcp{tool, 0.0, std::move(ps)};
    auto file = new GCDbgFile{std::move(gcp), color};
    file->setFileName(fileName);
    emit App::project().addFileDbg(file);
};

namespace GCode {

Creator::Creator() { }

void Creator::reset() {
    ProgressCancel::reset();

    file_ = nullptr;

    closedSrc = {};
    openSrcPaths.clear();
    returnPs.clear();
    returnPss.clear();
    supportPss.clear();
    groupedPss.clear();

    toolDiameter = 0.0;
    dOffset = 0.0;
    stepOver = 0.0;
}

Creator::~Creator() { ProgressCancel::reset(); }

std::vector<Geo::Polygon>& Creator::groupedPaths(Grouping group, double margin, bool skipFrame) {
    Timer t{"grouping"};

    groupedPss.clear();
    if(closedSrc.empty()) return groupedPss;

    if(group == Grouping::Copper) {
        // Медь -- это сам регион: его разбор на тела с отверстиями уже есть.
        groupedPss = closedSrc.all();
    } else {
        // Вырезы -- всё, что осталось от габаритной рамки после вычитания меди.
        // Прежде то же самое получалось объединением с рамкой по EvenOdd и
        // разбором дерева; здесь это прямая разность в точном домене.
        const Geo::Polygons frame{Geo::Polylines{boundingFrame(closedSrc.boundingRect(), margin)}};
        groupedPss = (frame - closedSrc).all();

        // Первым вырезом идёт «поле» между рамкой и деталью -- сама рамка с
        // деталью в качестве дырки. Оно не вырез, а артефакт рамки.
        if(!skipFrame && groupedPss.size() > 1)
            groupedPss.erase(groupedPss.begin());
    }

    return groupedPss;
}

void Creator::addRawPaths(Geo::Polylines&& rawPaths) {
    if(rawPaths.empty()) return;

    std::erase_if(rawPaths, [](const Geo::Polyline& path) { return path.size() < 2; });

    const double mergDist = App::project().glue();

    auto isClosed = [mergDist](const Geo::Polyline& path) {
        return path.closed
            || (path.size() > 3 && Geo::distance(path.front(), path.back()) < mergDist);
    };

    // Сперва отбираем уже замкнутое, потом пробуем склеить обрывки -- склейка
    // дорогая, и гонять её по тому, что и так замкнуто, незачем.
    Geo::Polylines closed;
    for(std::size_t i{}; i < rawPaths.size();)
        if(isClosed(rawPaths[i])) {
            closed.emplace_back(std::move(rawPaths[i])).close();
            rawPaths.erase(rawPaths.begin() + i);
        } else
            ++i;

    mergePolylines(rawPaths, mergDist);

    for(Geo::Polyline& path: rawPaths)
        if(isClosed(path))
            closed.emplace_back(std::move(path)).close();
        else
            openSrcPaths.emplace_back(std::move(path));

    if(!closed.empty())
        closedSrc |= Geo::Polygons{closed};
}

void Creator::createGc(Params&& newGcp) {
    qDebug(__FUNCTION__);

    reset();

    gcp = std::move(newGcp);

    closedSrc = gcp.closedCurves;
    addRawPaths(Geo::Polylines{gcp.openCurves});
    supportPss.append_range(gcp.supportCurvess);

    try {
        if(possibleTest() && !App::isDebug()) {
            try {
                checkMillingFl = true;
                checkMilling(gcp.side());
            } catch(const Cancel& e) {
                ProgressCancel::reset();
                qWarning() << u"checkMilling canceled:"_s << e.what();
            } catch(...) {
                throw;
            }
        }
        if(!isCancel()) {
            checkMillingFl = false;
            msg = tr("createGc");
            Timer t{"createGc"};
            create();
        }
        qWarning() << u"Creator finish"_s << file_;
    } catch(const Cancel& e) {
        qWarning() << u"Creator canceled:"_s << e.what();
    } catch(const std::exception& e) {
        qWarning() << u"Creator exeption:"_s << e.what();
    } catch(...) {
        qWarning() << u"Creator exeption:"_s << errno;
    }
    emit fileReady(file_);
}

File* Creator::file() const { return file_; }

std::pair<int, int> Creator::getProgress() {
    return {max(), current()};
}

void Creator::stacking(Geo::Polylines& paths) {
    qDebug(__FUNCTION__);

    if(paths.empty())
        return;
    Timer t{"stacking"};

    // Вложенность концентрических петель -- то, ради чего прежде строился
    // PolyTree с объединением по EvenOdd. Петли офсета друг друга не режут,
    // так что достаточно проверок принадлежности.
    const NestingForest forest = nestingForest(paths);
    returnPss.clear();

    // Направление фрезерования -- общее правило Params::reversedTravel плюс
    // поправка на дырку. Поправка нужна именно здесь: петли офсета приходят
    // все одной ориентации, и тело от дырки отличает не она, а чётность
    // вложенности. Там, где контуры приходят из точного домена в каноне
    // (Geo::Polygons::contours), о теле и дырке говорит сама ориентация, и
    // поправка не нужна -- см. Profile::Creator::orderContours.
    //
    // Прежняя запись `!(convent ^ !isHole) ^ (side == Outer)` -- то же самое:
    // !(c ^ !h) сводится к c ^ h, и всё выражение к (side == Outer) ^ c ^ h.
    auto orient = [this](Geo::Polyline& path, int depth) {
        const bool isHole = depth % 2 != 0;
        if(gcp.reversedTravel() ^ isHole)
            path.reverse();
    };

    std::function<void(std::size_t, bool)> walk = [&](std::size_t idx, bool newGroup) {
        Geo::Polyline path = paths[idx];
        orient(path, forest.depth[idx]);

        if(newGroup || returnPss.empty()) {
            returnPss.push_back({std::move(path)});
        } else {
            // Вложенная петля идёт тем же проходом, только если до неё не
            // дальше диаметра инструмента: иначе переезд всё равно подъёмом.
            const Geo::Polyline& prev = returnPss.back().back();
            double best = std::numeric_limits<double>::max();
            QPointF link;
            for(const Geo::Vertex& a: prev)
                for(const Geo::Vertex& b: path)
                    if(const double d = Geo::distance(a, b); d < best)
                        best = d, link = a;

            if(best <= toolDiameter) {
                rotateToNearest(path, link);
                returnPss.back().emplace_back(std::move(path));
            } else
                returnPss.push_back({std::move(path)});
        }

        for(std::size_t i{}; std::size_t child: forest.children[idx])
            walk(child, i++ != 0);
    };

    for(std::size_t root: forest.roots)
        walk(root, true);

    paths.clear();

    // Изнутри наружу: обход строился от объемлющих петель к вложенным, а
    // снимать материал начинают с внутренних.
    for(Geo::Polylines& group: returnPss)
        r::reverse(group);

    sortByProximity(returnPss, App::home().pos() + App::zero().pos());
}


static int condition{};

void Creator::isContinueCalc() {
    condition = 0;
    emit errorOccurred();
    std::unique_lock lk(mutex);
    cv.wait(lk, [] { return condition == 1; });
    items.clear();
    lk.unlock();
}

void Creator::continueCalc(bool fl) { // direct connection!!
    setCancel(!fl);
    condition = 1;
    cv.notify_all();
}

bool Creator::checkMilling(SideOfMilling side) {
    qDebug(__FUNCTION__);
    Timer t(__FUNCTION__);

    // Диаметр -- в миллиметрах: uScale ушёл вместе с целочисленным клиппером.
    const double toolDiameter = gcp.tools.back().getDiameter(gcp.getDepth());

    QString last{msg};
    msg = tr("Check milling for errors");

    // Морфологическое открытие: сжать на диаметр инструмента и раздуть обратно.
    // Что при этом не восстановилось -- туда фреза не входит. Обе операции
    // точные и идут, не выходя из CGAL, так что промежуточного огрубления нет.
    auto opening = [toolDiameter](const Geo::Polygons& region) {
        return Geo::Inflate(Geo::Inflate(region, -toolDiameter), +toolDiameter);
    };

    // За ошибку считается только то, что заметно больше пятна инструмента:
    // разница площадей описанного квадрата и самого пятна.
    const double testArea = toolDiameter * toolDiameter * (1.0 - std::numbers::pi / 4.0);

    auto report = [this](const Geo::Polygon& polygon) {
        items.push_back(new Gi::Error{polygon.contours(), polygon.area()});
    };

    switch(side) {
    case Outer: {
        Timer t{"Outer"};
        if(closedSrc.empty()) break;

        // Замыкание: раздуть и сжать обратно. Всё, что при этом ПРИБАВИЛОСЬ к
        // меди, -- материал в щелях уже самой фрезы, выбрать его нечем.
        const Geo::Polygons closing = Geo::Inflate(Geo::Inflate(closedSrc, +toolDiameter), -toolDiameter);
        const Geo::Polygons nonCut = closing - closedSrc;

        const auto& polygons = nonCut.all();
        setMax(polygons.size());
        setCurrent();
        for(const Geo::Polygon& polygon: polygons) {
            incCurrent();
            throwIfCancel();
            if(polygon.area() >= testArea)
                report(polygon);
        }
    } break;
    case Inner: {
        Timer t{"Inner"};
        groupedPaths(Grouping::Copper);

        setMax(groupedPss.size());
        setCurrent();
        for(const Geo::Polygon& group: groupedPss) {
            incCurrent();
            throwIfCancel();

            const Geo::Polygons body{group};
            const Geo::Polygons reachable = opening(body);

            if(reachable.empty()) // фреза не входит вовсе
                report(group);
            else if(reachable.size() > 1) // проход разрывается на куски
                for(const Geo::Polygon& polygon: (body - reachable).all())
                    if(polygon.area() >= testArea)
                        report(polygon);
        }
    } break;
    case On: break;
    }
    msg = last;

    if(items.size())
        isContinueCalc();
    return true;
}

Params Creator::getGcp() const { return gcp; }

void Creator::setGcp(const Params& newGcp) {
    gcp = newGcp;
    reset();
}

} // namespace GCode

// #include "moc_gc_creator.cpp"
