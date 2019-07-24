#ifndef GCODE_H
#define GCODE_H

#include <QGraphicsItemGroup>
#include <QObject>
#include <abstractfile.h>
#include <gbrvars.h>
#include <gi/itemgroup.h>
#include <myclipper.h>
#include <tooldatabase/tool.h>

enum GCodeType {
    Profile,
    Pocket,
    Voronoi,
    Thermal,
    Drill,
    GCodeProperties
};
namespace GCode {
class File : public AbstractFile {
    friend class PathItem;

public:
    File() {}
    File(const Paths& toolPaths, const Tool& tool, double depth, GCodeType type, const Paths& pocketPaths = {});

    Paths getPaths() const;
    void write(const QString& name);

    GCodeType gtype() const;

    FileType type() const override { return FileType::GCode; }
    Paths m_pocketPaths;

    QMap<int, QVector<int>> m_used;
    ItemGroup* itemGroup() const override { return m_itemGroup.data(); }

private:
    static QString lastDir;
    void saveDrill();
    void saveProfilePocket();

    GCodeType m_type;
    Paths m_g0path;
    Paths m_toolPaths;
    Tool m_tool;
    double m_depth;

    enum GCode {
        G_null = -1,
        G00 = 0,
        G01 = 1,
        G02 = 2,
        G03 = 3,
    };

    GCode m_gCode = G_null;

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

    inline QString x(double val) { return "X" + format(val); }
    inline QString y(double val) { return "Y" + format(val); }
    inline QString z(double val) { return "Z" + format(val); }
    inline QString feed(double val) { return "F" + format(val); }
    inline QString s(int val) { return "S" + QString::number(val); }
    inline QString format(double val) { return QString::number(val, 'f', 3); }

    void startPath(const QPointF& point);
    void endPath();
    void statFile();
    void endFile();

    QString formated(const QList<QString> data);

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

    bool FormatFlags[Size];

    QString lastValues[6];

    QList<QString> sl;

protected:
    virtual Paths merge() const { return m_toolPaths; }

    // AbstractFile interface
public:
    virtual void write(QDataStream& stream) const override;
    virtual void read(QDataStream& stream) override;

    static QString getLastDir();
    static void setLastDir(QString value);

    // AbstractFile interface
public:
    void createGi() override;
};
}
#endif // GCODE_H
