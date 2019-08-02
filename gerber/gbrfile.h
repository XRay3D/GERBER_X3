#ifndef GFILE_H
#define GFILE_H

#include "gbraperture.h"
#include "gbrvars.h"

#include <QDebug>
#include <abstractfile.h>
#include <gi/itemgroup.h>

namespace Gerber {

class File : public AbstractFile, public QList<GraphicObject> {
    friend class Parser;

public:
    File(const QString& name = "");
    ~File() override;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    enum ItemsType {
        Normal,
        Raw,
    };

    Format* format() { return &m_format; }
    Pathss& groupedPaths(Group group = CopperGroup, bool fl = false);
    bool flashedApertures() const;
    const QMap<int, QSharedPointer<AbstractAperture>>* apertures() const;
    void setItemType(ItemsType type);
    void setRawItemGroup(ItemGroup* itemGroup);
    ItemGroup* rawItemGroup() const;
    ItemsType itemsType() const;
    FileType type() const override { return FileType::Gerber; }
    ItemGroup* itemGroup() const override;
    void addToScene() const ;
    void setColor(const QColor& color);

protected:
    Paths merge() const override;

private:
    QMap<int, QSharedPointer<AbstractAperture>> m_apertures;
    ItemsType m_itemsType = Normal;
    QSharedPointer<ItemGroup> m_rawItemGroup;
    void grouping(PolyNode* node, Pathss* pathss, Group group);
    Format m_format;
    Layer layer = Copper;
    Miror miror = Vertical;
    QVector<int> rawIndex;

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
