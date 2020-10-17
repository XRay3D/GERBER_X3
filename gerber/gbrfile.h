/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "gbraperture.h"
#include "gbrcomponent.h"
#include "gbrtypes.h"

#include "abstractfile.h"
#include <QDebug>
#include <forward_list>
#include <gi/itemgroup.h>
namespace Gerber {

class File : public AbstractFile, public QList<GraphicObject> {
    friend class Parser;

public:
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
    const ApertureMap* apertures() const;
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

    ApertureMap m_apertures;
    ItemsType m_itemsType = Normal;
    void grouping(PolyNode* node, Pathss* pathss, Group group);
    Format m_format;
    //Layer layer = Copper;
    //Miror miror = Vertical;
    QVector<int> rawIndex;
    std::forward_list<Path> checkList;

    // AbstractFile interface
protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

    // AbstractFile interface
public:
    void createGi() override;
    const QList<Component>& components() const;
};
}

Q_DECLARE_METATYPE(Gerber::File)
