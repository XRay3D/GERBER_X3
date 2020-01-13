#ifndef GFILE_H
#define GFILE_H

#include "gbraperture.h"
#include "gbrcomponent.h"
#include "gbrtypes.h"

#include <QDebug>
#include <abstractfile.h>
#include <gi/itemgroup.h>

namespace Gerber {

class File : public AbstractFile, public QList<GraphicObject> {
    friend class Parser;

public:
    explicit File(QDataStream& stream);
    explicit File(const QString& name = "");
    ~File() override;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    enum ItemsType {
        Normal,
        ApPaths,
        Components,
    };

    Format* format() { return &m_format; }
    Pathss& groupedPaths(Group group = CopperGroup, bool fl = false);
    bool flashedApertures() const;
    const QMap<int, QSharedPointer<AbstractAperture>>* apertures() const;
    void setItemType(ItemsType type);
    ItemGroup* itemGroup(ItemsType type) const;
    ItemsType itemsType() const;
    FileType type() const override { return FileType::Gerber; }
    ItemGroup* itemGroup() const override;
    void addToScene() const;
    void setColor(const QColor& color);

protected:
    Paths merge() const override;

private:
    QList<Component> m_components;

    QMap<int, QSharedPointer<AbstractAperture>> m_apertures;
    ItemsType m_itemsType = Normal;
    void grouping(PolyNode* node, Pathss* pathss, Group group);
    Format m_format;
    //Layer layer = Copper;
    //Miror miror = Vertical;
    QVector<int> rawIndex;
    QList<QSharedPointer<Path>> checkList;

    // AbstractFile interface
public:
    virtual void write(QDataStream& stream) const override;
    virtual void read(QDataStream& stream) override;

    // AbstractFile interface
public:
    void createGi() override;
};
}

Q_DECLARE_METATYPE(Gerber::File)

#endif // GFILE_H
