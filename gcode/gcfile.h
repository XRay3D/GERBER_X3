#pragma once
#ifndef GCODE_H
#define GCODE_H

#include "gctypes.h"
#include <abstractfile.h>

class Project;

namespace GCode {

class File : public AbstractFile {
    friend class ::PathItem;
    friend class ::Project;

public:
    explicit File(QDataStream& stream) { read(stream); } // for project load
    explicit File(const Pathss& toolPathss, const GCodeParams& gcp, const Paths& pocketPaths = {});
    void save(const QString& name);
    GCodeType gtype() const;
    FileType type() const override { return FileType::GCode; }
    ItemGroup* itemGroup() const override { return m_itemGroup.last(); }

private:
    static QString lastDir;
    static const QVector<QChar> cmdList;

    Paths m_g0path;

    ////////////////////////////////////////
    Paths m_pocketPaths; /////
    Pathss m_toolPathss; /////
    const GCodeParams m_gcp; ////

    const double feedRate = 0.0;
    const double plungeRate = 0.0;
    const int spindleSpeed = 0.0;
    const int toolType = 0;

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

    bool formatFlags[Size];
    QString lastValues[6];
    QList<QString> m_gCodeText;
    Code m_gCode = G_null;

    inline QString g0()
    {
        m_gCode = G00;
        return "G0";
    }

    inline QString g1()
    {
        m_gCode = G01;
        return "G1";
    }

    inline QString x(double val) { return 'X' + format(val); }
    inline QString y(double val) { return 'Y' + format(val); }
    inline QString z(double val) { return 'Z' + format(val); }
    inline QString feed(double val) { return 'F' + format(val); }
    inline QString speed(int val) { return 'S' + QString::number(val); }
    inline QString format(double val)
    {
        QString str(QString::number(val, 'g', (abs(val) < 1 ? 3 : (abs(val) < 10 ? 4 : (abs(val) < 100 ? 5 : 6)))));
        if (str.contains('e'))
            return QString::number(val, 'f', 3);
        return str;
    }

    QVector<double> getDepths();
    QVector<QVector<QPolygonF>> pss(const QPointF& offset);
    QVector<QPolygonF> ps(const QPointF& offset);

    void initSave();
    void genGcodeAndTile();
    void addInfo(bool fl = false);
    void startPath(const QPointF& point);
    void endPath();
    void statFile();
    void endFile();

    QString formated(const QList<QString>& data);

    void createGiDrill();
    void createGiPocket();
    void createGiProfile();
    void createGiRaster();
    void createGiLaser();

    void saveDrill(const QPointF& offset);
    void savePocket(const QPointF& offset);
    void saveProfile(const QPointF& offset);
    void saveLaser(const QPointF& offset);

    // AbstractFile interfaces
protected:
    virtual Paths merge() const override { return {}; }

public:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    void createGi() override;

    static QString getLastDir();
    static void setLastDir(QString value);
    QList<QString> gCodeText() const;
    Tool getTool() const;
    GCodeParams gcp() const;
};

}
#endif // GCODE_H
