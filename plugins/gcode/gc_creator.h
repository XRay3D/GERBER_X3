/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

// #include "abstract_file.h"
#include "gc_types.h"
#include "myclipper.h"
// #include "utils.h"

#include <QObject>

#include <condition_variable>
#include <mutex>
#include <sstream>

#include <ranges>
namespace ranges = std::ranges;
namespace rviews = std::ranges::views;

// #if __has_include(<source_location>)
//     #include <source_location>
// using sl = std::source_location;
// #else
//     #include <experimental/source_location>
// using sl = std::experimental::source_location;
// #endif

void dbgPaths(Paths ps, const QString& fileName, QColor color = Qt::red, bool closed = false, const Tool& tool = {0.});

inline void dbgPaths(Pathss pss, const QString& fileName, QColor color = Qt::red, bool closed = false, const Tool& tool = {0.}) {
    if (pss.empty())
        return;
    for (auto&& paths : pss.midRef(1))
        pss.front().append(std::move(paths));
    dbgPaths(pss.front(), fileName, color, closed, tool);
}

class GiError;

namespace GCode {

class File;

class Creator : public QObject, public ProgressCancel {
    Q_OBJECT

public:
    Creator();
    void reset();
    //    Creator(const Paths& workingPaths, const bool convent, SideOfMilling side);
    ~Creator() override;

    File* file() const;

    std::pair<int, int> getProgress();

    void addPaths(Paths&& paths);
    void addRawPaths(Paths paths);
    void addSupportPaths(Pathss supportPaths);

    Pathss& groupedPaths(Grouping group, Point::Type offset = uScale, bool skipFrame = {});
    void grouping(Grouping group, PolyTree& node);

    Path boundOfPaths(const Paths& paths, Point::Type k) const;

    /*static*/ Paths& sortB(Paths& src);
    /*static*/ Paths& sortBeginEnd(Paths& src);

    /*static*/ Pathss& sortB(Pathss& src);
    /*static*/ Pathss& sortBeginEnd(Pathss& src);

    void createGc();

    void continueCalc(bool fl = true);

    Params getGcp() const;
    void setGcp(const Params& gcp_);

    //    static void //PROG .3setProgMax(int progressMax);
    //    static void //PROG //PROG .3setProgMaxAndVal(int progressMax, int progressVal);
    //    static void //PROG setProgInc();

    QString msg;

    mvector<GiError*> items;

    bool checkMillingFl {};

signals:
    void fileReady(File* file);
    void canceled();
    void errorOccurred(int = 0);

protected:
    bool checkMilling(SideOfMilling side);

    void stacking(Paths& paths);

    /////////////////////////////////////////////////
    /// \brief склеивает пути при совпадении конечных точек
    /// \param paths - пути
    /// \param maxDist - максимальное расстояние между конечными точками
    void mergeSegments(Paths& paths, double maxDist = 0.0);

    void mergePaths(Paths& paths, const double maxDist = 0.0);

    void markPolyTreeDByNesting(PolyTree& polynode);
    void sortPolyTreeByNesting(PolyTree& polynode);

    std::unordered_map<void*, int> nesting;

    virtual void create() = 0;
    virtual uint32_t type() = 0;

    //    inline static ClipperBase* clipperPtr_;
    //    inline static bool cancel_;
    //    static inline int //PROG  progressMax_;
    //    static inline int //PROG progressVal_;

    File* file_ = nullptr;
    Paths workingPs;
    Paths workingRawPs;
    Paths returnPs;
    Pathss returnPss;
    Pathss supportPss;
    Pathss groupedPss;

    double toolDiameter {};
    double dOffset {};
    Point::Type stepOver {};
    Params gcp_;

    void isContinueCalc();

private:
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace GCode

#include "app.h"
