/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "gctypes.h"
#include "myclipper.h"
#include <QMutex>
#include <QObject>
#include <QWaitCondition>
//#include <QThread>
//#include <QThreadPool>
//#include <QtConcurrent>
//#include <tooldatabase/tool.h>

using namespace ClipperLib2;

void dbgPaths(Paths ps, const QString& fileName, bool closed = false, const Tool& tool = { 1 });

class ErrorItem;

namespace GCode {

class File;

class Creator : public QObject {
    Q_OBJECT

public:
    Creator() { }
    void reset();
    //    Creator(const Paths& workingPaths, const bool convent, SideOfMilling side);
    ~Creator() override;

    File* file() const;

    QPair<int, int> getProgress();

    void addRawPaths(Paths rawPaths);
    void addSupportPaths(Pathss supportPaths);
    void addPaths(const Paths& paths);

    Pathss& groupedPaths(Grouping group, cInt k = uScale);

    static Paths& sortB(Paths& src);
    static Paths& sortBE(Paths& src);

    static Pathss& sortB(Pathss& src);
    static Pathss& sortBE(Pathss& src);

    void createGc();
    void createGc(const GCodeParams& gcp);

    void cancel();

    void proceed();

    GCodeParams getGcp() const;
    void setGcp(const GCodeParams& gcp);

    static void progress(int progressMax);
    static void progress(int progressMax, int progressVal);
    static void progress();
    QString msg;

    QVector<ErrorItem*> items;

signals:
    void fileReady(GCode::File* file);
    void canceled();
    void errorOccurred(int = 0);

protected:
    bool createability(bool side);

    bool pointOnPolygon(const QLineF& l2, const Path& path, Point64* ret = nullptr);
    void stacking(Paths& paths);
    void mergeSegments(Paths& paths, double glue = 0.0);

    virtual void create() = 0;
    virtual GCodeType type() = 0;

    inline static bool m_cancel;
    inline static int m_progressMax;
    inline static int m_progressVal;

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
