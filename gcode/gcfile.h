#ifndef GCODE_H
#define GCODE_H

#include "gcvars.h"
//#include <QGraphicsItemGroup>
//#include <QObject>
#include <abstractfile.h>
//#include <gbrvars.h>
//#include <gi/itemgroup.h>
//#include <myclipper.h>
//#include <tooldatabase/tool.h>

namespace GCode {
class File : public AbstractFile {
    friend class ::PathItem;

public:
    explicit File(QDataStream& stream) { read(stream); } // for project load
    explicit File(const Pathss& toolPathss, const Tool& tool, double depth, GCodeType type, const Paths& pocketPaths = {});
    void save(const QString& name);
    GCodeType gtype() const;
    FileType type() const override { return FileType::GCode; }
    Paths m_pocketPaths;
    QMap<int, QVector<int>> m_usedItems;
    ItemGroup* itemGroup() const override { return m_itemGroup.data(); }

private:
    static QString lastDir;
    void saveDrill();
    void savePocket();
    void saveProfile();

    QVector<QVector<QPolygonF>> pss();
    QVector<QPolygonF> ps();

    QVector<double> getDepths();

    GCodeType m_type;
    Paths m_g0path;
    Pathss m_toolPathss;
    Tool m_tool;
    double m_depth;

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

    inline QString x(double val) { return "X" + format(val); }
    inline QString y(double val) { return "Y" + format(val); }
    inline QString z(double val) { return "Z" + format(val); }
    inline QString feed(double val) { return "F" + format(val); }
    inline QString s(int val) { return "S" + QString::number(val); }
    inline QString format(double val)
    {
        QString str(QString::number(val, 'g', (abs(val) < 1 ? 3 : (abs(val) < 10 ? 4 : (abs(val) < 100 ? 5 : 6)))));
        if (str.contains('e'))
            return QString::number(val, 'f', 3);
        return str;
    }

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
    void createGiDrill();
    void createGiPocket();
    void createGiProfile();
    void createGiRaster();

protected:
    virtual Paths merge() const override { return Paths(); /*m_toolPaths;*/ }

    // AbstractFile interface
public:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    void createGi() override;

    static QString getLastDir();
    static void setLastDir(QString value);
};
}
#endif // GCODE_H
