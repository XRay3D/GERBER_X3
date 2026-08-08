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

class GcFileProxy;

// Пересобрать lines_ у всех G-code файлов проекта. Вызывать сразу же, как только
// меняется параметр, влияющий на генерацию текста G-кода (сторона платы, safeZ/
// clearence/plunge, положение Home/Zero/Pin и т.п.) — иначе lines_ останется
// устаревшим до следующего явного "Save Toolpath".
void regenerateGCodeFiles();

class File : public AbstractFile {
    // friend class ::Project;
    friend class GcFileProxy;

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

    // Пересобрать lines_ (текст G-кода) из текущих параметров: стороны платы,
    // безопасной высоты, зазора, глубины подвода и т.п. Нужно вызывать сразу же,
    // как только один из этих параметров меняется у уже созданного файла —
    // иначе lines_ останется устаревшим до следующего явного "Save Toolpath".
    void regenerate();

    static void ensureDefaultScripts();

    void initSave();
    void statFile();
    void addInfo();
    virtual void genGcodeAndTile() = 0;
    void endFile();

    FileTree::Node* node() override;

protected:
    void startPath(const QPointF& point);
    void endPath();

    std::vector<Geo::Polylines> mirrorAndOffsetCurves(const QPointF& offset);
    Geo::Polylines mirrorAndOffsetCurves(const QPointF& offset, Geo::Polylines paths_);

    std::vector<QSharedPointer<QColor>> debugColor;

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

    Geo::Polylines g0path_;
    double z_{};

    static inline QString lastDir;
    static inline bool redirected;
    static inline constexpr auto CMD_LIST = u"GXYZIJSF"_sv;

    std::vector<double> getDepths();

    bool formatFlags[Size]{};
    QString lastValues[SpaceG /*6*/];
    Code gCode_ = GNull;

    std::vector<QString> savePath(const Geo::Polyline& curve, double perimeter = {}, double depth = {});

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
    void extracted(const Geo::Vertex& v);
    QString g(const Geo::Vertex& v);

    // QString i(double val) { return u'I' + format(val); }
    // QString j(double val) { return u'J' + format(val); }
    // QString x(double val) { return u'X' + format(val); }
    // QString y(double val) { return u'Y' + format(val); }
    // QString z(double val) { return u'Z' + format(val); }
    // QString speed(int val) { return u'S' + QString::number(val); }

    static QString format(double val);

    bool runJsScript(const QString& scriptPath);

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
