/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
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

#include "interfaces/file.h"
#include "itemgroup.h"

#include <QDebug>
#include <forward_list>

namespace Gerber {

class File : public FileInterface {
    friend class Parser;
    friend class Plugin;

public:
    explicit File(const QString& name = "");
    ~File() override;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    Format* format() { return &m_format; }
    Pathss& groupedPaths(Group group = CopperGroup, bool fl = false);
    bool flashedApertures() const;
    const ApertureMap* apertures() const;

    enum ItemsType {
        NullType = -1,
        Normal,
        ApPaths,
        Components,
    };
    void setItemType(int type) override;
    int itemsType() const override;
    void initFrom(FileInterface* file) override;

    FileType type() const override { return FileType::Gerber; }

    void addToScene() const;
    void setColor(const QColor& color) override;
    mvector<const AbstrGraphicObject*> graphicObjects() const override;

protected:
    Paths merge() const override;

private:
    QList<Component> m_components;
    mvector<GraphicObject> m_graphicObjects;
    ApertureMap m_apertures;
    void grouping(PolyNode* node, Pathss* pathss, Group group);
    Format m_format;

    //Layer layer = Copper;
    //Miror miror = Vertical;
    //QPointf offset;

    QVector<int> rawIndex;
    std::forward_list<Path> checkList;

    // FileTree::Node interface
protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

    // FileTree::Node interface
public:
    void createGi() override;
    const QList<Component>& components() const;
};
}

Q_DECLARE_METATYPE(Gerber::File)
