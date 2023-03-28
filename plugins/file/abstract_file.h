/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "datastream.h"

// #include "app.h"
#include "ft_node.h"
#include "gi_group.h"
// #include "myclipper.h"
#include "plugintypes.h"
#include "splashscreen.h"

#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QModelIndex>

enum FileType {
    Gerber_,
    Excellon_,
    Dxf_,
    Hpgl,
    TopoR,

    GCode_ = 100,
    // 101 - ...

    Shapes_ = 200
    // 201 - ...
};

Q_DECLARE_METATYPE(FileType)

using LayerTypes = std::vector<LayerType>;

template <typename T>
inline AbstractFile* load(QDataStream& stream) {
    auto* file = new T;
    stream >> *file;
    return file;
}

class AbstractFile {
    friend QDataStream& operator<<(QDataStream& stream, const AbstractFile& file) {
        stream << static_cast<int>(file.type());
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        Block(out).write(
            file.id_,
            file.colorFlag_,
            file.color_,
            file.date_,
            file.groupedPaths_,
            file.itemsType_,
            file.layerTypes_,
            file.lines_,
            file.mergedPaths_,
            file.name_,
            file.side_,
            file.transform_,
            file.isVisible());
        file.write(out);
        return stream << data;
    }

    friend QDataStream& operator>>(QDataStream& stream, AbstractFile& file) {
        QByteArray data;
        stream >> data;
        QDataStream in(&data, QIODevice::ReadOnly);
        bool visible;
        Block(in).read(
            file.id_,
            file.colorFlag_,
            file.color_,
            file.date_,
            file.groupedPaths_,
            file.itemsType_,
            file.layerTypes_,
            file.lines_,
            file.mergedPaths_,
            file.name_,
            file.side_,
            file.transform_,
            visible);
        file.read(in);

        if (App::splashScreen())
            App::splashScreen()->showMessage(QObject::tr("Preparing: ") + file.shortName() + "\n\n\n", Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
        file.createGi();
        file.setTransform(file.transform_);
        file.setVisible(visible);
        return stream;
    }

public:
    struct Transform {
        double angle {};
        QPointF translate {};
        QPointF scale {1, 1};

        friend QDataStream& operator<<(QDataStream& stream, const Transform& tr) {
            return Block(stream).write(tr);
        }
        friend QDataStream& operator>>(QDataStream& stream, Transform& tr) {
            return Block(stream).read(tr);
        }

        operator QTransform() const {
            QTransform t;
            t.translate(translate.x(), translate.y());
            t.rotate(angle);
            t.scale(scale.x(), scale.y());
            return t;
        }
    };

    AbstractFile()
        : itemGroups_ {new GiGroup} { }

    virtual ~AbstractFile() { qDeleteAll(itemGroups_); }

    QString shortName() const { return QFileInfo(name_).fileName(); }
    QString name() const { return name_; }
    void setFileName(const QString& fileName) { name_ = fileName; }

    void addToScene() const {
        for (const auto var : itemGroups_) {
            if (var && var->size()) {
                var->addToScene();
                var->setZValue(-id_);
            }
        }
    }

    GiGroup* itemGroup(int type = -1) const {
        const int size(static_cast<int>(itemGroups_.size()));
        if (type == -1 && 0 <= itemsType_ && itemsType_ < size)
            return itemGroups_[itemsType_];
        else if (0 <= type && type < size)
            return itemGroups_[type];
        return itemGroups_.front();
    }

    const mvector<GiGroup*>& itemGroups() const { return itemGroups_; }

    Paths mergedPaths() const { return mergedPaths_.size() ? mergedPaths_ : merge(); }
    Pathss groupedPaths() const { return groupedPaths_; }

    mvector<QString>& lines() { return lines_; }
    const mvector<QString>& lines() const { return lines_; }
    const QString lines2() const {
        QString rstr;
        for (auto&& str : lines_)
            rstr.append(str).append('\n');
        return rstr;
    }
    virtual mvector<const AbstrGraphicObject*> graphicObjects() const { return {}; }

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    virtual void initFrom(AbstractFile* file) {
        id_ = file->id();
        node_ = file->node();
        // node_->setId(&id_);
    }

    virtual FileType type() const = 0;
    const LayerTypes& displayedTypes() const { return layerTypes_; }

    virtual void createGi() = 0;
    //    virtual void selected() = 0;

    virtual void setItemType([[maybe_unused]] int type) {};
    virtual int itemsType() const { return itemsType_; };

    Side side() const { return side_; }
    void setSide(Side side) { side_ = side; }

    virtual bool isVisible() const { return (visible_ = itemGroup()->isVisible()); }
    virtual void setVisible(bool visible) { itemGroup()->setVisible(visible_ = visible); }

    const QColor& color() const { return color_; }
    virtual void setColor(const QColor& color) { color_ = color; }

    void setTransform([[maybe_unused]] const Transform& transform) {
        transform_ = transform;
        QTransform t {transform};
        for (auto* ig : itemGroups_)
            for (auto* gi : *ig)
                gi->setTransform(t);
    }
    const auto& transform() const { return transform_; }

    const int& id() const { return id_; }
    void setId(int id) { id_ = id; }

    virtual FileTree::Node* node() = 0;
    virtual QIcon icon() const { return {}; }

    bool userColor() const { return colorFlag_; }
    void setUserColor(bool userColor) { colorFlag_ = userColor; }

protected:
    virtual void write(QDataStream& stream) const = 0;
    virtual void read(QDataStream& stream) = 0;
    virtual Paths merge() const = 0;

    LayerTypes layerTypes_;
    FileTree::Node* node_ = nullptr;
    Pathss groupedPaths_;
    QColor color_;
    bool colorFlag_ {};
    QDateTime date_;
    QString name_;
    Side side_ = Top;
    int id_ = -1;
    int itemsType_ = -1;
    mutable Paths mergedPaths_;
    mutable bool visible_ = false;
    mvector<GiGroup*> itemGroups_;
    mvector<QString> lines_;
    //    QTransform transform_;
    Transform transform_;
};

#define FileInterface_iid "ru.xray3d.XrSoft.GGEasy.AbstractFile"

Q_DECLARE_INTERFACE(AbstractFile, FileInterface_iid)
