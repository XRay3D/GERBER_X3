/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "fileifce.h"
#include "gc_types.h"
#include "myclipper.h"

#include <QObject>

#include <condition_variable>
#include <mutex>
#include <sstream>

// #if __has_include(<source_location>)
//     #include <source_location>
// using sl = std::source_location;
// #else
//     #include <experimental/source_location>
// using sl = std::experimental::source_location;
// #endif

void dbgPaths(Paths ps, const QString& fileName, QColor color = Qt::red, bool closed = false, const Tool& tool = {0.});

class GiError;

class ProgressCancel {
    static inline int max_;
    static inline int current_;
    static inline bool cancel_;

public:
    static void reset() {
        current_ = 0;
        max_ = 0;
        cancel_ = 0;
    }

    /////////////////
    /// \brief Progress max
    /// \return
    ///
    static int max() { return max_; }
    /////////////////
    /// \brief Progress setMax
    /// \param max
    ///
    static void setMax(int max) { max_ = max; }

    /////////////////
    /// \brief Progress current
    /// \return
    ///
    static int current() { return current_; }
    /////////////////
    /// \brief Progress setCurrent
    /// \param current
    ///
    static void setCurrent(int current = 0) { current_ = current; }
    /////////////////
    /// \brief Progress incCurrent
    ///
    static void incCurrent() { ++current_; }
    static bool isCancel() { return cancel_; }
    static void ifCancelThenThrow(/*const sl location = sl::current()*/) {
        ++current_;
        if (cancel_) [[unlikely]] {
            //            static std::stringstream ss;
            //            ss.clear();
            //            ss << "file: "
            //               << location.file_name() << "("
            //               << location.line() << ":"
            //               << location.column() << ") `"
            //               << location.function_name();
            //            throw cancelException(ss.str().data() /*__FUNCTION__*/);
            throw cancelException(__FUNCTION__);
        }
    }
    static void setCancel(bool cancel) { cancel_ = cancel; }
};

inline void ifCancelThenThrow() { ProgressCancel::ifCancelThenThrow(); }

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

    void addRawPaths(Paths rawPaths);
    void addSupportPaths(Pathss supportPaths);
    void addPaths(const Paths& paths);

    Pathss& groupedPaths(Grouping group, Point::Type k = uScale, bool fl = {});

    Path boundPaths(const Paths& paths, Point::Type k) const;

    /*static*/ Paths& sortB(Paths& src);
    /*static*/ Paths& sortBE(Paths& src);

    /*static*/ Pathss& sortB(Pathss& src);
    /*static*/ Pathss& sortBE(Pathss& src);

    void createGc();

    void continueCalc(bool fl = true);

    GCodeParams getGcp() const;
    void setGcp(const GCodeParams& gcp_);

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

    bool pointOnPolygon(const QLineF& l2, const Path& path, Point* ret = nullptr);
    void stacking(Paths& paths);
    void mergeSegments(Paths& paths, double glue = 0.0);

    void mergePaths(Paths& paths, const double dist = 0.0);

    void markPolyTreeDByNesting(PolyTree& polynode);
    void sortPolyTreeByNesting(PolyTree& polynode);

    std::unordered_map<void*, int> nesting;

    virtual void create() = 0;
    virtual GCodeType type() = 0;

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
    GCodeParams gcp_;

    void isContinueCalc();

private:
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace GCode

#include "app.h"
