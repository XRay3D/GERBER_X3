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

#include "geo/boolean.h"
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
        regenerate();
    }
    void serialize(Serial::Writer& /*sb*/) const override { } // отладочный, не сохраняется
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
    // AbstractFile interface
};

void dbgPaths(Geo::Polylines ps, QAnyStringView fileName, QColor color, bool close, const Tool& tool) {
    std::erase_if(ps, [](const Geo::Polyline& path) { return path.size() < 2; });
    if(ps.empty())
        return;
    if(close)
        r::for_each(ps, [](Geo::Polyline& p) { p.close(); });
    GCode::Params gcp{tool, 0.0, std::move(ps)};
    auto file = new GCDbgFile{std::move(gcp), color};
    file->setFileName(fileName.toString());
    emit App::project().addFileDbg(file);
};

namespace GCode {

Creator::Creator() { }

void Creator::reset() {
    Progress::reset();

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

Creator::~Creator() { Progress::reset(); }

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

// Габарит всего, из чего строится УП. Открытые пути идут наравне с
// замкнутыми: профиль по разомкнутой линии -- такая же траектория.
QRectF Creator::sourceExtent() const {
    QRectF extent = closedSrc.boundingRect();
    for(const Geo::Polyline& path: openSrcPaths)
        extent = extent.united(path.boundingRect());
    return extent;
}

void Creator::addRawPaths(Geo::Polylines&& rawPaths) {
    if(rawPaths.empty()) return;

    // Чертёж приходит без канона: ориентация обхода произвольна, а контур
    // разрезан на сущности. Geo::normalize склеивает обрывки по клею проекта и
    // собирает замкнутое по EVEN-ODD -- прежний Geo::Polygons{closed} читал
    // контур, обойдённый по часовой, как пустоту, и тот пропадал вовсе.
    auto [region, open] = Geo::normalize(std::move(rawPaths), App::project().glue());

    openSrcPaths.append_range(std::move(open));
    if(!region.empty()) closedSrc |= region;
}

void Creator::createGc(Params&& newGcp, std::stop_token token) {
    qDebug(__FUNCTION__);

    // Отмена -- на весь расчёт разом: под этой областью её видят и наши
    // проверки (Geo::checkCancelled), и сами операции Geo, включая рабочие
    // потоки, которые они заводят себе внутри.
    Geo::CancelScope cancelScope{std::move(token)};

    reset();

    gcp = std::move(newGcp);

    closedSrc = gcp.closedCurves;
    addRawPaths(Geo::Polylines{gcp.openCurves});
    supportPss.append_range(gcp.supportCurvess);

    try {
        // Габарит проверяется ПЕРВЫМ делом: координаты УП пишутся целыми (см.
        // GCode::Units), и то, что в них не влезло, переполнилось бы молча --
        // фреза уехала бы на другой конец стола. Считать такое незачем.
        if(const QRectF extent = sourceExtent(); !fitsOutput(extent))
            throw std::runtime_error{
                u"Габарит %1 x %2 мм не представим в выводе"_s
                    .arg(extent.width())
                    .arg(extent.height())
                    .toStdString()};

        if(possibleTest() && !App::isDebug()) {
            checkMillingFl = true;
            checkMilling(gcp.side());
            checkMillingFl = false;
        }
        // «Break» в таблице ошибок отменяет весь прогон, а не одну проверку.
        Geo::checkCancelled();
        setMsg(tr("createGc"));
        Timer t{"createGc"};
        create();
        qWarning() << u"Creator finish"_s << file_;
    } catch(const Geo::Cancelled&) {
        qWarning() << u"Creator canceled"_s;
    } catch(const std::exception& e) {
        qWarning() << u"Creator exeption:"_s << e.what();
    } catch(...) {
        qWarning() << u"Creator exeption:"_s << errno;
    }
    checkMillingFl = false;
    // Файл отдают всегда -- хоть готовый, хоть пустой: форма считает по нему
    // законченные инструменты и без него прогон не закроет. Отменённый прогон
    // она узнаёт сама и присланное выбрасывает.
    emit fileReady(file_);
}

File* Creator::file() const { return file_; }

void Creator::setMsg(const QString& text) {
    std::lock_guard lk{msgMutex};
    msg_ = text;
}

QString Creator::message() const {
    std::lock_guard lk{msgMutex};
    return msg_;
}

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

    // Направление фрезерования задаёт САМ контур. Петли приходят из точного
    // домена в каноне (Geo::Polygons::contours: внешняя граница против часовой,
    // дырки по часовой), а канон здесь и есть ПОПУТНЫЙ ход: материал по нему
    // всегда справа -- у стенки кармана он снаружи петли, у острова внутри.
    // Встречный ход -- разворот всех петель разом, и сторона фрезерования на
    // это не влияет: её роль (Params::reversedTravel) уже сыграна тем, с какой
    // стороны от детали построена сама область.
    //
    // Чётность вложенности, которую брали прежде, для этого не годится:
    // концентрические петли лежат одна в другой, и глубина растёт с каждым
    // витком офсета -- направление чередовалось бы от прохода к проходу, и
    // попутным выходил бы лишь каждый второй. Ср. Profile::Creator::
    // orderContours: там на каждую границу приходится один контур, и общее
    // правило Params::reversedTravel работает как есть.
    auto orient = [convent = gcp.convent()](Geo::Polyline& path) {
        if(convent) path.reverse();
    };

    std::function<void(std::size_t, bool)> walk = [&](std::size_t idx, bool newGroup) {
        Geo::Polyline path = paths[idx];
        orient(path);

        if(newGroup || returnPss.empty()) {
            returnPss.push_back({std::move(path)});
        } else {
            // Вложенная петля идёт тем же проходом, только если до неё не
            // дальше диаметра инструмента: иначе переезд всё равно подъёмом.
            // Мерится расстояние от вершин вложенной до КОНТУРА объемлющей:
            // у офсетных петель это ровно шаг, где бы ни стояли вершины.
            // Прежняя мерка вершина-вершина рвала заливку круглой дырки на
            // несколько проходов -- две вершины окружности после каждого
            // офсета оказываются где придётся.
            const Geo::Polyline& prev = returnPss.back().back();
            double best = std::numeric_limits<double>::max();
            for(const Geo::Vertex& b: path)
                best = std::min(best, closestPoint(prev, b).distance);

            if(best <= toolDiameter)
                returnPss.back().emplace_back(std::move(path));
            else
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
    //
    // Начало каждой следующей петли -- в ближайшей к концу предыдущей ТОЧКЕ
    // контура (замкнутая петля кончается там, где началась). Переезд между
    // витками тогда ровно шаг офсета, по нерасчищенному ещё телу кармана,
    // и скрипт ведёт его подачей, не поднимая фрезу. Крутить надо именно в
    // порядке фрезерования: сдвиг, сделанный при обходе от объемлющей к
    // вложенной, для обратного хода бесполезен -- он ставил начало
    // ВЛОЖЕННОЙ, а ехать предстоит от неё к объемлющей.
    for(Geo::Polylines& group: returnPss) {
        r::reverse(group);
        for(std::size_t i = 1; i < group.size(); ++i)
            rotateToClosest(group[i], group[i - 1].front());
    }

    sortByProximity(returnPss, App::home().pos() + App::zero().pos());
}

void Creator::isContinueCalc() {
    {
        std::lock_guard lk{mutex};
        answered_ = false;
    }
    emit errorOccurred(); // очередью в GUI-поток: там покажут таблицу ошибок
    std::unique_lock lk{mutex};
    cv.wait(lk, [this] { return answered_; });
    items.clear();
}

// Мьютекс тут обязателен: без него «answered_ = true» может лечь между
// проверкой предиката и засыпанием, и пробуждение потеряется -- расчётный
// поток уснёт навсегда, а с ним и весь прогон.
//
// Отмену («Break») этот вызов НЕ делает -- её просят у стоп-токена, и делает
// это форма перед тем, как разбудить: проснувшийся поток упрётся в ближайшую
// Geo::checkCancelled и свернётся сам.
void Creator::continueCalc() {
    {
        std::lock_guard lk{mutex};
        answered_ = true;
    }
    cv.notify_all();
}

bool Creator::checkMilling(SideOfMilling side) {
    qDebug(__FUNCTION__);
    Timer t(__FUNCTION__);

    // Диаметр -- в миллиметрах: uScale ушёл вместе с целочисленным клиппером.
    const double toolDiameter = gcp.tools.back().getDiameter(gcp.getDepth());

    const QString last{message()};
    setMsg(tr("Check milling for errors"));

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
            Geo::checkCancelled();
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
            Geo::checkCancelled();

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
    setMsg(last);

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
