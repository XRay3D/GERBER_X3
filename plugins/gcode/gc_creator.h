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

#include "gc_types.h"
#include "gi_error.h"
#include "myclipper.h"

#include <QObject>

#include <cancelation.h>
#include <condition_variable>
#include <mutex>

namespace ranges = std::ranges;
namespace rviews = std::ranges::views;

// namespace Gi {
// class Error;
// }

void dbgPaths(Paths ps, const QString& fileName, QColor color = Qt::red, bool closed = false, const Tool& tool = {0.});

inline void dbgPaths(Pathss pss, const QString& fileName, QColor color = Qt::red, bool closed = false, const Tool& tool = {0.}) {
    if(pss.empty())
        return;
    for(auto&& paths: pss.midRef(1))
        pss.front().insert(pss.front().end(), paths.begin(), paths.end());
    dbgPaths(pss.front(), fileName, color, closed, tool);
}

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

    Pathss& groupedPaths(Grouping group, /*Point::Type*/ int32_t offset = uScale, bool skipFrame = {});
    void grouping(Grouping group, PolyTree& node);

    Path boundOfPaths(const Paths& paths, /*Point::Type*/ int32_t k) const;

    void createGc(Params* gcp);

    void continueCalc(bool fl = true);

    //    static void //PROG .3setProgMax(int progressMax);
    //    static void //PROG //PROG .3setProgMaxAndVal(int progressMax, int progressVal);
    //    static void //PROG setProgInc();

    QString msg;

    mvector<Gi::Error*> items;

    bool checkMillingFl{};

private:
    void addRawPaths(Paths&& paths);

    Params getGcp() const;
    void setGcp(const Params& gcp_);

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
    // void mergePaths(Paths& paths, const double maxDist = 0.0);

    void markPolyTreeDByNesting(PolyTree& polynode);
    void sortPolyTreeByNesting(PolyTree& polynode);

    std::unordered_map<void*, int> nesting;

    virtual void create() { }             /* = 0; */
    virtual uint32_t type() { return 0; } /* = 0; */
    virtual bool possibleTest() const { return false; }

    //    inline static ClipperBase* clipperPtr_;
    //    inline static bool cancel_;
    //    static inline int //PROG  progressMax_;
    //    static inline int //PROG progressVal_;

    File* file_ = nullptr;
    Paths closedSrcPaths;
    Paths openSrcPaths;
    Paths returnPs;
    Pathss returnPss;
    Pathss supportPss;
    Pathss groupedPss;

    double toolDiameter{};
    double dOffset{};
    /*Point::Type*/ int32_t stepOver{};
    Params gcp_;

    void isContinueCalc();

private:
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace GCode

#include "app.h"
