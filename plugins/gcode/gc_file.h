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

#include "fileifce.h"
#include "gc_types.h"
#include <QList>
#include <QString>

class Project;

namespace GCode {

struct GCObj : QObject { };

class File : public FileInterface {
    //    friend class ::Project;

public:
    File(GCodeParams&& gcp, Paths&& pocketPaths, Pathss&& toolPathss);
    File();
    //    GCodeType gtype() const;

    mvector<QString> gCodeText() const;
    Tool getTool() const;
    const GCodeParams& gcp() const;

    double feedRate();
    double plungeRate();
    int spindleSpeed();
    int toolType();

    void setFeedRate(double val);
    void setPlungeRate(double val);
    void setSpindleSpeed(int val);
    void setToolType(int val);

    static QString getLastDir();
    static void setLastDir(QString dirPath);

    bool save(const QString& name);

    void initSave();
    void statFile();
    void addInfo();
    virtual void genGcodeAndTile() = 0;
    void endFile();

private:
    double feedRate_ {};
    double plungeRate_ {};
    int spindleSpeed_ {};
    int toolType_ {};

protected:
    void startPath(const QPointF& point);
    void endPath();

    mvector<mvector<QPolygonF>> normalizedPathss(const QPointF& offset);

    mvector<QPolygonF> normalizedPaths(const QPointF& offset, const Paths& paths_);

    ////////////////////////////////////////
    Paths pocketPaths_; /////
    Pathss toolPathss_; /////
    mvector<QSharedPointer<QColor>> debugColor;

    GCodeParams gcp_; ////
    enum {
        AlwaysG,
        AlwaysX,
        AlwaysY,
        AlwaysZ,
        AlwaysF,
        AlwaysS,

        SpaceG,
        SpaceX,
        SpaceY,
        SpaceZ,
        SpaceF,
        SpaceS,

        Size
    };

    Paths g0path_;

    static inline QString lastDir;
    static inline bool redirected;
    inline static const mvector<QChar> cmdList {'G', 'X', 'Y', 'Z', 'F', 'S'};

    mvector<double> getDepths();

    bool formatFlags[Size];
    QString lastValues[6];
    Code gCode_ = GNull;

    mvector<QString> savePath(const QPolygonF& path, double spindleSpeed);

    QString formated(const mvector<QString>& data);

    QString g0();
    QString g1();

    QString x(double val);
    QString y(double val);
    QString z(double val);

    QString feed(double val);
    QString speed(int val);
    QString format(double val);

    virtual Paths merge() const override { return {}; }

    // FileInterface interfaces
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    void initFrom(FileInterface* file) override { qWarning(__FUNCTION__); }
    FileTree::Node* node() override;

    /////////////////////////////////////////////////////////////

    void saveDrill(const QPointF& offset);
    void saveLaserHLDI(const QPointF& offset);
    void saveLaserPocket(const QPointF& offset);
    void saveLaserProfile(const QPointF& offset);
    void saveMillingPocket(const QPointF& offset);
    void saveMillingProfile(const QPointF& offset);
    void saveMillingRaster(const QPointF& offset);

    /////////////////////////////////////////////////////////////

    void createGiDrill();
    void createGiLaser();
    void createGiPocket();
    void createGiProfile();
    void createGiRaster();
};

} // namespace GCode
