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

    // Имя, которое дал программе пользователь (текст поля Name в форме), без
    // суффикса инструмента. Отдельно от name_, потому что тот работает на две
    // ставки: до сохранения это метка в дереве, после save() -- путь на диске.
    // Опираться на name_ при поиске коллизий и при восстановлении формы нельзя.
    QString programName_;

    // Видимость файлов проекта на момент расчёта: id файла -> был ли видим.
    // Снимок, а не список использованных: восстановить надо ровно ту картину,
    // которая была перед нажатием Create, включая погашенные файлы.
    std::map<int32_t, bool> visibility_;

public:
    File(Params&& newGcp)
        : feedRate{newGcp.feedRate()}
        , plungeRate{newGcp.plungeRate()}
        , spindleSpeed{newGcp.spindleSpeed()}
        , toolType{newGcp.toolType()}
        , strFeed{u'F' + formatValue(feedRate)}
        , strPlungeFeed{u'F' + formatValue(plungeRate)}
        , strSpindle{u'S' + formatValue(spindleSpeed)}
        , gcp{std::move(newGcp)} {
        setSide(gcp.params[Params::FileSide]);
    }

    File() = default;
    ~File() override = default;

    std::vector<QString> gCodeText() const { return lines_; }
    const Tool& tool() const { return gcp.tool(); }
    const Params& params() const { return gcp; }

    const QString& programName() const { return programName_; }
    void setProgramName(const QString& name) { programName_ = name; }

    const std::map<int32_t, bool>& visibility() const { return visibility_; }
    void setVisibility(std::map<int32_t, bool> visibility) { visibility_ = std::move(visibility); }

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
    void addSourceInfo();
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
    // Из них координатные -- те, что живут на сетке вывода (GCode::Units).
    static inline constexpr auto COORD_LIST = u"XYZIJ"_sv;

    std::vector<double> getDepths();

    bool formatFlags[Size]{};
    Code gCode_ = GNull;

    // Слово УП: буква и то, что за ней. Координатные несут с собой своё ЧИСЛО
    // в единицах вывода -- повтор подавляется по нему, а не по тексту: текст
    // это лишь запись числа, и решать по записи значит решать по округлению.
    struct Word {
        QString text;
        Units units{};
        bool numeric{};
        Word(QString text)
            : text{std::move(text)} { }
        Word(QChar letter, Units units)
            : text{letter + format(units)}
            , units{units}
            , numeric{true} { }
        operator const QString&() const { return text; }
    };

    // Последнее НАПИСАННОЕ значение каждого слова: у координатных числом, у
    // прочих (G, S, F -- там значение и есть текст) текстом.
    Units lastUnits[SpaceG /*6*/]{};
    bool lastWritten[SpaceG]{};
    QString lastValues[SpaceG];

    std::vector<QString> savePath(const Geo::Polyline& curve, double perimeter = {}, double depth = {});

    QString formated(const std::vector<Word>& data);
    // Готовые слова из JS-скрипта. Координатные разбираются обратно в число
    // и печатаются заново -- чтобы вывод скрипта шёл по той же сетке и с тем
    // же подавлением повторов, что и вывод C++.
    QString formatedText(const std::vector<QString>& data);

    struct Coord {
        QChar c;
        Word operator()(double mm) const { return {c, toUnits(mm)}; }
        operator Word() const { return {c, Units{}}; }
    } static constexpr i{u'I'}, j{u'J'}, x{u'X'}, y{u'Y'}, z{u'Z'};

    // Обороты шпинделя длиной не являются -- своя запись, не по сетке вывода.
    struct {
        QChar c;
        Word operator()(double val) const { return {c + formatValue(val)}; }
    } static constexpr speed{u'S'};

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

    // Текст координаты -- ровно запись её числа: цифры до точки, значащие
    // после и ни одного хвостового нуля.
    static QString format(Units units);
    static QString format(double mm) { return format(toUnits(mm)); }

    // Число, длиной НЕ являющееся: подача (мм/мин), обороты. Сетка вывода
    // задана допуском геометрии, и к минутам с оборотами он отношения не
    // имеет -- у них своя запись, до трёх знаков без хвостовых нулей.
    static QString formatValue(double val);

    bool runJsScript(const QString& scriptPath);

    // AbstractFile interfaces
    void write(QDataStream& stream) const override { stream << gcp << programName_ << visibility_; }
    void read(QDataStream& stream) override { stream >> gcp >> programName_ >> visibility_; }
    // initFrom не переопределяется: базовая реализация переносит ровно то, что
    // нужно при замене УП на месте (id, узел дерева, сторону, цвет, тип, трансформ),
    // а programName_ и visibility_ переносить как раз НЕЛЬЗЯ -- у нового файла они
    // уже свои, выставленные формой до замены.
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
