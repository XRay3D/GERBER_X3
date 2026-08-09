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
#pragma once

#include "gc_pathutils.h"
#include "gc_types.h"
#include "gi_error.h"

#include "geo/boolean.h"
#include "geo/util.h"

#include <QObject>

#include "geo/cancel.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <progress.h>
#include <stop_token>

namespace ranges = r;
namespace v = r::views;

// namespace Gi {
// class Error;
// }

void dbgPaths(Geo::Polylines ps,
    const QString& fileName,
    QColor color = Qt::red,
    bool closed = false,
    const Tool& tool = {0.});

inline void dbgPaths(std::vector<Geo::Polylines> pss, const QString& fileName, QColor color = Qt::red, bool closed = false, const Tool& tool = {0.}) {
    if(pss.empty())
        return;
    for(auto&& paths: pss | v::drop(1))
        pss.front().insert(pss.front().end(), paths.begin(), paths.end());
    dbgPaths(pss.front(), fileName, color, closed, tool);
}

namespace GCode {

class File;

class Creator : public QObject, public Progress {
    Q_OBJECT

public:
    Creator();
    void reset();
    // Creator(const Paths& workingPaths, const bool convent, SideOfMilling side);
    ~Creator() override;

    File* file() const;

    std::pair<int, int> getProgress();

    // Разбор исходного региона на тела с отверстиями. PolyTree для этого больше
    // не нужен: Geo::Polygons и ЕСТЬ разобранная вложенность -- медь это его
    // собственные полигоны, а вырезы -- полигоны того, что осталось от рамки
    // после вычитания меди.
    std::vector<Geo::Polygon>& groupedPaths(Grouping group, double margin = 1.0, bool skipFrame = {});

    // Весь расчёт целиком. Зовётся из РАБОЧЕГО потока, а token -- его же
    // стоп-токен: под него ставится Geo::CancelScope, и отмена начинает
    // действовать на все операции Geo этого потока, включая те, что идут
    // внутри одного вызова (см. geo/cancel.h). Без токена считает до конца.
    void createGc(Params&& gcp, std::stop_token token = {});

    // Ответ на вопрос, заданный isContinueCalc: расчётный поток спит, пока
    // GUI не нажмёт «Continue» или «Break». Зовётся ПРЯМО из GUI-потока и
    // только будит; отменяет («Break») стоп-токен, а не этот вызов.
    void continueCalc();

    // Надпись в прогрессбаре. Пишет её расчётный поток, читает GUI -- отсюда
    // мьютекс, а не голое поле: QString неявно разделяемая, и гонка на ней --
    // не мусор в надписи, а падение на счётчике ссылок.
    void setMsg(const QString& text);
    QString message() const;

    std::vector<Gi::Error*> items;

    // Идёт проверка фрезеруемости. Ставит расчётный поток, читает GUI.
    std::atomic_bool checkMillingFl{};

private:
    QRectF sourceExtent() const;
    void addRawPaths(Geo::Polylines&& paths);

    Params getGcp() const;
    void setGcp(const Params& gcp);

signals:
    void fileReady(File* file);
    void errorOccurred(int = 0);

protected:
    bool checkMilling(SideOfMilling side);

    // Раскладывает замкнутые петли по группам «одна врезка -- один проход»:
    // вложенные друг в друга петли, отстоящие не дальше диаметра инструмента,
    // идут одной цепочкой и разворачиваются так, чтобы переезд между ними был
    // кратчайшим. Результат -- в returnPss.
    void stacking(Geo::Polylines& paths);

    virtual void create() { }             /* = 0; */
    virtual uint32_t type() { return 0; } /* = 0; */
    virtual bool possibleTest() const { return false; }

    // inline static ClipperBase* clipperPtr_;
    // inline static bool cancel_;
    // static inline int //PROG  progressMax_;
    // static inline int //PROG progressVal_;

    File* file_ = nullptr;
    // Замкнутое -- регион в точном домене, открытое -- просто набор линий:
    // площади у линии нет, и в CGAL ей делать нечего.
    Geo::Polygons closedSrc;
    Geo::Polylines openSrcPaths;
    Geo::Polylines returnPs;
    std::vector<Geo::Polylines> returnPss;
    std::vector<Geo::Polylines> supportPss;
    std::vector<Geo::Polygon> groupedPss;

    double toolDiameter{};
    double dOffset{};
    double stepOver{};
    Params gcp;

    void isContinueCalc();

private:
    // Ожидание ответа на найденные ошибки: расчётный поток спит на cv, GUI
    // будит его из continueCalc. Поля именно ЭТОГО Creator'а, а не общие на
    // всех: форм с расчётом может идти несколько разом.
    std::mutex mutex;
    std::condition_variable cv;
    bool answered_{};

    mutable std::mutex msgMutex;
    QString msg_;
};

} // namespace GCode

// #include "app.h"
