#ifndef TOOLPATHCREATOR_H
#define TOOLPATHCREATOR_H

#include "gcvars.h"
#include <QObject>
#include <QThread>
#include <myclipper.h>
#include <tooldatabase/tool.h>

using namespace ClipperLib;

namespace GCode {

class File;

class Creator : public QObject {
    Q_OBJECT
    friend class ClipperLib::Clipper;
    friend class ClipperLib::ClipperOffset;

public:
    static Creator* self;
    Creator() {}
    //    Creator(const Paths& workingPaths, const bool convent, SideOfMilling side);
    ~Creator() override;
    virtual void create(const GCodeParams& gcp) = 0;
    Pathss& groupedPaths(Grouping group, cInt k = 10, bool fl = true);
    void addRawPaths(Paths rawPaths);
    void addSupportPaths(Pathss supportPaths);
    void addPaths(const Paths& paths);

    File* file() const;

    int progressMax() const { return m_progressMax; }
    int progressValue() const;
    Paths& sortByStratDistance(Paths& src);
    Paths& sortByStratEndDistance(Paths& src);
    bool PointOnPolygon(const QLineF& l2, const Path& path, IntPoint* ret = nullptr);

signals:
    void fileReady(GCode::File* file);

protected:
    inline void progressOrCancel(int progressMax, int progressValue)
    {
        if (progressValue != 0)
            m_progressValue = progressValue;
        if (progressMax != 0)
            m_progressMax = progressMax;
        if (!(progressValue % 100)) {
            if (QThread::currentThread()->isInterruptionRequested())
                throw true;
        }
    }
    static void progressOrCancel();

    File* m_file = nullptr;
    Paths m_workingPaths;
    Paths m_workingRawPaths;
    Paths m_returnPaths;
    Pathss m_supportPathss;
    Pathss m_groupedPathss;
    void grouping(PolyNode* node, Pathss* pathss, Grouping group);

    Path& fixPath(PolyNode* node);
    void grouping2(PolyNode* node, Paths* paths, bool fl = false);
    void grouping3(Paths& paths);
    void mergeSegments(Paths& paths);

    int m_progressMax = 1000000;
    int m_progressValue = 0;

    //    /*const*/ SideOfMilling m_side;
    double m_toolDiameter = 0.0;
    double m_dOffset = 0.0;
    double m_stepOver = 0.0;
    //    /*const*/ bool m_convent;

    GCodeParams m_gcp;
};
}

#endif // TOOLPATHCREATOR_H
