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

#include "abstract_file.h"
#include "gc_types.h"

#include <QList>
#include <QString>

class Project;

namespace GCode {

class File : public AbstractFile {
    // friend class ::Project;
protected:
    double feedRate{};
    double plungeRate{};
    int spindleSpeed{};
    int toolType{};
    QString strFeed;
    QString strPlungeFeed;
    QString strSpindle;
    Params gcp; ////

public:
    File(Params&& newGcp)
        : feedRate{newGcp.feedRate()}
        , plungeRate{newGcp.plungeRate()}
        , spindleSpeed{newGcp.spindleSpeed()}
        , toolType{newGcp.toolType()}
        , strFeed{u'F' + format(feedRate)}
        , strPlungeFeed{u'F' + format(plungeRate)}
        , strSpindle{u'S' + format(spindleSpeed)}
        , gcp{std::move(newGcp)} {
        setSide(gcp.params[Params::FileSide]);
    }

    File() = default;
    ~File() override = default;

    std::vector<QString> gCodeText() const { return lines_; }
    const Tool& tool() const { return gcp.tool(); }

    static QString getLastDir();
    static void setLastDir(QString dirPath);

    bool save(const QString& name);

    void initSave();
    void statFile();
    void addInfo();
    virtual void genGcodeAndTile() = 0;
    void endFile();

    FileTree::Node* node() override;

protected:
    void startPath(const QPointF& point);
    void endPath();

    Curvess mirrorAndOffsetCurves(const QPointF& offset);
    Curves mirrorAndOffsetCurves(const QPointF& offset, Curves paths_);

    mvector<QSharedPointer<QColor>> debugColor;

    enum {
        AlwaysG,
        AlwaysX,
        AlwaysY,
        AlwaysZ,
        AlwaysI,
        AlwaysJ,
        AlwaysS,
        AlwaysF,

        SpaceG,
        SpaceX,
        SpaceY,
        SpaceZ,
        SpaceI,
        SpaceJ,
        SpaceS,
        SpaceF,

        Size
    };

    Curves g0path_;
    double z_{};

    static inline QString lastDir;
    static inline bool redirected;
    static inline constexpr auto CMD_LIST = u"GXYZIJSF"_sv;

    mvector<double> getDepths();

    bool formatFlags[Size]{};
    QString lastValues[SpaceG /*6*/];
    Code gCode_ = GNull;

    std::vector<QString> savePath(const Curve& curve, double perimetr = {}, double depth = {});

    QString formated(const std::vector<QString>& data);

    struct {
        QChar c;
        constexpr QString operator()(double val) const { return c + format(val); }
        constexpr operator QString() const { return c + u"0"_s; }
    } static constexpr i{u'I'}, j{u'J'}, x{u'X'}, y{u'Y'}, z{u'Z'}, speed{u'S'};

    QString g0();
    QString g1();
    QString g2();
    QString g3();
    void extracted(const geo::Vertex& v);
    QString g(const geo::Vertex& v);

    // QString i(double val) { return u'I' + format(val); }
    // QString j(double val) { return u'J' + format(val); }
    // QString x(double val) { return u'X' + format(val); }
    // QString y(double val) { return u'Y' + format(val); }
    // QString z(double val) { return u'Z' + format(val); }
    // QString speed(int val) { return u'S' + QString::number(val); }

    static QString format(double val);

    virtual Paths merge() const override { return {}; }

    // AbstractFile interfaces
    void write(QDataStream& stream) const override { stream << gcp; }
    void read(QDataStream& stream) override { stream >> gcp; }
    void initFrom(AbstractFile* /*file*/) override { qWarning(__FUNCTION__); }
    // FileTree::Node* node() override;

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
