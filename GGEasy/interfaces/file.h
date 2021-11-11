/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "datastream.h"

#include "pluginfile.h"
#include "plugintypes.h"

#include "itemgroup.h"
#include "splashscreen.h"
#include <ft_node.h>
#include <myclipper.h>

#include "app.h"

#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QModelIndex>

#include "leakdetector.h"

using LayerTypes = std::vector<LayerType>;

class FileInterface {

    //    friend class Project;
    //    friend QDataStream& operator<<(QDataStream& stream, const QSharedPointer<FileInterface>& file);
    //    friend QDataStream& operator>>(QDataStream& stream, QSharedPointer<FileInterface>& file);

    friend QDataStream& operator<<(QDataStream& stream, const FileInterface& file)
    {
        stream << static_cast<int>(file.type());
        file.write(stream);
        stream << file.m_id;
        stream << file.m_itemsType;
        stream << file.m_lines;
        stream << file.m_name;
        stream << file.m_mergedPaths;
        stream << file.m_groupedPaths;
        stream << file.m_side;
        stream << file.m_color;
        stream << file.m_userColor;
        stream << file.m_date;
        stream << file.isVisible();
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, FileInterface& file)
    {
        file.read(stream);
        stream >> file.m_id;
        stream >> file.m_itemsType;
        stream >> file.m_lines;
        stream >> file.m_name;
        stream >> file.m_mergedPaths;
        stream >> file.m_groupedPaths;
        stream >> file.m_side;
        stream >> file.m_color;
        stream >> file.m_userColor;
        stream >> file.m_date;
        if (App::splashScreen())
            App::splashScreen()->showMessage(QObject::tr("Preparing: ") + file.shortName() + "\n\n\n", Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
        bool visible;
        stream >> visible;
        file.createGi();
        file.setVisible(visible);
        return stream;
    }

public:
    FileInterface()
        : m_itemGroups(1, new ItemGroup)
    {
    }
    virtual ~FileInterface() { qDeleteAll(m_itemGroups); }

    QString shortName() const { return QFileInfo(m_name).fileName(); }
    QString name() const { return m_name; }
    void setFileName(const QString& fileName) { m_name = fileName; }

    void addToScene() const
    {
        for (const auto var : m_itemGroups) {
            if (var && var->size()) {
                var->addToScene();
                var->setZValue(-m_id);
            }
        }
    }

    ItemGroup* itemGroup(int type = -1) const
    {
        const int size(static_cast<int>(m_itemGroups.size()));
        if (type == -1 && 0 <= m_itemsType && m_itemsType < size)
            return m_itemGroups[m_itemsType];
        else if (0 <= type && type < size)
            return m_itemGroups[type];
        return m_itemGroups.front();
    }

    const mvector<ItemGroup*>& itemGroups() const { return m_itemGroups; }

    Paths mergedPaths() const { return m_mergedPaths.size() ? m_mergedPaths : merge(); }
    Pathss groupedPaths() const { return m_groupedPaths; }

    mvector<QString>& lines() { return m_lines; }
    const mvector<QString>& lines() const { return m_lines; }
    const QString lines2() const
    {
        QString rstr;
        for (auto&& str : m_lines)
            rstr.append(str).append('\n');
        return rstr;
    }
    virtual mvector<const AbstrGraphicObject*> graphicObjects() const { return {}; }

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    virtual void initFrom(FileInterface* file)
    {
        m_id = file->id();
        m_node = file->node();
        m_node->setId(&m_id);
    }

    virtual FileType type() const = 0;
    const LayerTypes& displayedTypes() const { return m_layerTypes; }

    virtual void createGi() = 0;
    //    virtual void selected() = 0;

    virtual void setItemType(int type) { Q_UNUSED(type) };
    virtual int itemsType() const { return m_itemsType; };

    Side side() const { return m_side; }
    void setSide(Side side) { m_side = side; }

    virtual bool isVisible() const { return (m_visible = itemGroup()->isVisible()); }
    virtual void setVisible(bool visible) { itemGroup()->setVisible(m_visible = visible); }

    const QColor& color() const { return m_color; }
    virtual void setColor(const QColor& color) { m_color = color; }

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    virtual FileTree::Node* node() = 0;

    bool userColor() const { return m_userColor; }
    void setUserColor(bool userColor) { m_userColor = userColor; }

protected:
    virtual void write(QDataStream& stream) const = 0;
    virtual void read(QDataStream& stream) = 0;
    virtual Paths merge() const = 0;

    LayerTypes m_layerTypes;
    FileTree::Node* m_node = nullptr;
    Pathss m_groupedPaths;
    QColor m_color;
    bool m_userColor {};
    QDateTime m_date;
    QString m_name;
    Side m_side = Top;
    int m_id = -1;
    int m_itemsType = -1;
    mutable Paths m_mergedPaths;
    mutable bool m_visible = false;
    mvector<ItemGroup*> m_itemGroups;
    mvector<QString> m_lines;
};

#define FileInterface_iid "ru.xray3d.XrSoft.GGEasy.FileInterface"

Q_DECLARE_INTERFACE(FileInterface, FileInterface_iid)
