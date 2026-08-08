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

#include "gc_types.h"
#include "gi_error.h"

#include <QObject>

#include <cancelation.h>
#include <condition_variable>
#include <mutex>

namespace ranges = r;
namespace v = r::views;

// namespace Gi {
// class Error;
// }

void dbgPaths(Paths64 ps,
    const QString& fileName,
    QColor color = Qt::red,
    bool closed = false,
    const Tool& tool = {0.});

inline void dbgPaths(Pathss64 pss, const QString& fileName, QColor color = Qt::red, bool closed = false, const Tool& tool = {0.}) {
    if(pss.empty())
        return;
    for(auto&& paths: pss | v::drop(1))
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
    // Creator(const Paths& workingPaths, const bool convent, SideOfMilling side);
    ~Creator() override;

    File* file() const;

    std::pair<int, int> getProgress();

    Pathss64& groupedPaths(Grouping group, /*PType*/ int32_t offset = uScale, bool skipFrame = {});
    void grouping(Grouping group, PolyTree& node);

    void createGc(Params&& gcp);

    void continueCalc(bool fl = true);

    // static void //PROG .3setProgMax(int progressMax);
    // static void //PROG //PROG .3setProgMaxAndVal(int progressMax, int progressVal);
    // static void //PROG setProgInc();

    QString msg;

    mvector<Gi::Error*> items;

    bool checkMillingFl{};

private:
    void addRawPaths(Paths64&& paths);

    Params getGcp() const;
    void setGcp(const Params& gcp);

signals:
    void fileReady(File* file);
    void canceled();
    void errorOccurred(int = 0);

protected:
    bool checkMilling(SideOfMilling side);

    void stacking(Paths64& paths);

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

    // inline static ClipperBase* clipperPtr_;
    // inline static bool cancel_;
    // static inline int //PROG  progressMax_;
    // static inline int //PROG progressVal_;

    File* file_ = nullptr;
    Paths64 closedSrcPaths;
    Paths64 openSrcPaths;
    Paths64 returnPs;
    Pathss64 returnPss;
    Pathss64 supportPss;
    Pathss64 groupedPss;

    double toolDiameter{};
    double dOffset{};
    /*PType*/ int32_t stepOver{};
    Params gcp;

    void isContinueCalc();

private:
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace GCode

// #include "app.h"
