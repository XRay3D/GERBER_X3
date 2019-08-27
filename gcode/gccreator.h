#ifndef TOOLPATHCREATOR_H
#define TOOLPATHCREATOR_H

#include "gcvars.h"
#include <QObject>
#include <QThread>
#include <QThreadPool>
#include <QtConcurrent>
#include <myclipper.h>
#include <tooldatabase/tool.h>

using namespace ClipperLib;

namespace GCode {

class File;

class Creator : public QObject {
    Q_OBJECT

public:
    static Creator* self;
    Creator() {}
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
    void createGc(const GCodeParams& gcp);

    void cancel() { m_cancel = true; }

    GCodeParams getGcp() const;
    void setGcp(const GCodeParams& gcp);

    static void progress(int progressMax);
    static void progress(int progressMax, int progressVal);
    static void progress();
signals:
    void fileReady(GCode::File* file);

protected:
    bool pointOnPolygon(const QLineF& l2, const Path& path, IntPoint* ret = nullptr);
    void stacking(Paths& paths);
    void mergeSegments(Paths& paths, double glue = 0.0);

    virtual void create(const GCodeParams& gcp) = 0;

    static bool m_cancel;
    static int m_progressMax;
    static int m_progressVal;

    File* m_file = nullptr;
    Paths m_workingPs;
    Paths m_workingRawPs;
    Paths m_returnPs;
    Pathss m_returnPss;
    Pathss m_supportPss;
    Pathss m_groupedPss;

    double m_toolDiameter = 0.0;
    double m_dOffset = 0.0;
    double m_stepOver = 0.0;
    GCodeParams m_gcp;
};
}

#endif // TOOLPATHCREATOR_H
