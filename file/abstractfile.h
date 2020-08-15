#pragma once



#include <QDateTime>
#include <QFileInfo>
#include <datastream.h>
#include <gi/itemgroup.h>
#include <myclipper.h>

using namespace ClipperLib;

enum class FileType {
    Gerber,
    Excellon,
    GCode,
};

enum Side {
    NullSide = -1,
    Top,
    Bottom
};

class AbstractFile {
    friend class Project;
    friend QDataStream& operator<<(QDataStream& stream, const QSharedPointer<AbstractFile>& file);
    friend QDataStream& operator>>(QDataStream& stream, QSharedPointer<AbstractFile>& file);

public:
    AbstractFile();
    virtual ~AbstractFile();

    QString shortName() const;
    QString name() const;
    void setFileName(const QString& name);

    virtual ItemGroup* itemGroup() const = 0;

    Paths mergedPaths() const;
    Pathss groupedPaths() const;

    QList<QString>& lines();

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    virtual FileType type() const = 0;
    virtual void write(QDataStream& stream) const = 0;
    virtual void read(QDataStream& stream) = 0;
    virtual void createGi() = 0;
    //    virtual void selected() = 0;

    Side side() const;
    void setSide(Side side);

    QColor color() const;
    void setColor(const QColor& color);

    int id() const;

protected:
    int m_id = -1;
    virtual Paths merge() const = 0;

    QVector<ItemGroup*> m_itemGroup;
    QString m_name;
    QList<QString> m_lines;
    mutable Paths m_mergedPaths;
    Pathss m_groupedPaths;

    Side m_side = Top;
    QColor m_color;
    QDateTime m_date;

    void _write(QDataStream& stream) const;
    void _read(QDataStream& stream);
};

//Q_DECLARE_METATYPE(AbstractFile)


