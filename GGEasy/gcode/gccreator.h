/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "gctypes.h"
#include <QMutex>
#include <QObject>
#include <QWaitCondition>
#include <myclipper.h>
//#include <QThread>
//#include <QThreadPool>
//#include <QtConcurrent>
//#include "tool.h"
#include <source_location>

using namespace ClipperLib;

void dbgPaths(Paths ps, const QString& fileName, bool closed = false, const Tool& tool = { 1 });

class ErrorItem;

//namespace GCode {
//class Creator;
//}

class ProgressCancel {
    static inline size_t m_max = 0;
    static inline size_t m_current = 0;
    static inline bool m_cancel = false;
    //    static inline GCode::Creator* m_creator = nullptr;
    static inline ClipperLib::ClipperBase* m_clipper = nullptr;

public:
    ProgressCancel() { }

    static void reset()
    {
        m_max = 0;
        m_current = 0;
        m_cancel = false;
        //        m_creator = nullptr;
        m_clipper = nullptr;
    }

    //    static GCode::Creator* creator() { return m_creator; }
    //    static void setCreator(GCode::Creator* creator) { m_creator = creator; }

    static void setClipper(ClipperLib::ClipperBase* clipper) { m_clipper = clipper; }
    static ClipperLib::ClipperBase* clipper() { return m_clipper; }

    static size_t getMax() { return m_max; }
    static void setMax(size_t max) { m_max = max; }

    static size_t getCurrent() { return m_current; }
    static void setCurrent(size_t current = 0) { m_current = current; }
    static void incCurrent() { ++m_current; }

    static bool getCancel() { return m_cancel; }
    static void ifCancelThenThrow(const std::source_location location = std::source_location::current())
    {
        static std::stringstream ss;
        if (m_cancel) {
            ss.clear();
            ss << "file: "
               << location.file_name() << "("
               << location.line() << ":"
               << location.column() << ") `"
               << location.function_name();
            throw cancelException(ss.str().data());
        }
    }
    static void setCancel(bool cancel) { m_cancel = cancel; }
};

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

    Pathss& groupedPaths(Grouping group, cInt k = uScale);

    /*static*/ Paths& sortB(Paths& src);
    /*static*/ Paths& sortBE(Paths& src);

    /*static*/ Pathss& sortB(Pathss& src);
    /*static*/ Pathss& sortBE(Pathss& src);

    void createGc();

    void cancel();
    void proceed();

    GCodeParams getGcp() const;
    void setGcp(const GCodeParams& gcp);

    //    static void //PROG .3setProgMax(int progressMax);
    //    static void //PROG //PROG .3setProgMaxAndVal(int progressMax, int progressVal);
    //    static void //PROG setProgInc();

    QString msg;

    mvector<ErrorItem*> items;

signals:
    void fileReady(GCode::File* file);
    void canceled();
    void errorOccurred(int = 0);

protected:
    bool createability(bool side);

    bool pointOnPolygon(const QLineF& l2, const Path& path, IntPoint* ret = nullptr);
    void stacking(Paths& paths);
    void mergeSegments(Paths& paths, double glue = 0.0);

    void mergePaths(Paths& paths, const double dist = 0.0);

    void markPolyNodeByNesting(PolyNode& polynode);
    void sortPolyNodeByNesting(PolyNode& polynode);

    virtual void create() = 0;
    virtual GCodeType type() = 0;

    //    inline static ClipperBase* m_clipperPtr;
    //    inline static bool m_cancel;
    //    static inline int //PROG  m_progressMax;
    //    static inline int //PROG m_progressVal;

    File* m_file = nullptr;
    Paths m_workingPs;
    Paths m_workingRawPs;
    Paths m_returnPs;
    Pathss m_returnPss;
    Pathss m_supportPss;
    Pathss m_groupedPss;

    double m_toolDiameter = 0.0;
    double m_dOffset = 0.0;
    cInt m_stepOver = 0;
    GCodeParams m_gcp;

    void isContinueCalc();

private:
    QMutex mutex;
    QWaitCondition condition;
};

} // namespace GCode

#include "app.h"
