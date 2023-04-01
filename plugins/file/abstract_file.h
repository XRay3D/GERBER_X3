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
#include "gi_group.h"
#include "md5.h"
#include "plugintypes.h"

#include <QDateTime>
#include <QPainter>
#include <QSplashScreen>

inline QPixmap decoration(QColor color, QChar chr = {}) {

    QPixmap pixmap(22, 22);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    color.setAlpha(255);
    p.setBrush(color);
    p.drawRect(2, 2, 18, 18);
    if (!chr.isNull()) {
        QFont f;
        f.setBold(true);
        f.setPixelSize(18);
        p.setFont(f);
        // p.setPen(Qt::white);
        p.drawText(QRect(2, 2, 18, 18), Qt::AlignCenter, {chr});
    }
    return pixmap;
}

using LayerTypes = std::vector<LayerType>;

namespace FileTree {
class Node;
}

class AbstractFile {

    friend QDataStream& operator<<(QDataStream& stream, const AbstractFile& file) {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        Block(out).write(
            file.id_,
            file.date_,
            file.groupedPaths_,
            file.itemsType_,
            file.layerTypes_,
            file.lines_,
            file.mergedPaths_,
            file.name_,
            file.side_,
            file.transform_,
            file.isVisible(),
            file.color_,
            file.colorFlag_);
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
            file.date_,
            file.groupedPaths_,
            file.itemsType_,
            file.layerTypes_,
            file.lines_,
            file.mergedPaths_,
            file.name_,
            file.side_,
            file.transform_,
            visible,
            file.color_,
            file.colorFlag_);
        file.read(in);
        if (App::splashScreen())
            App::splashScreen()->showMessage(QObject::tr("Preparing: ") + file.shortName() + "\n\n\n", Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
        file.createGi();
        file.setTransform(file.transform_);
        file.setVisible(visible);
        return stream;
    }

public:
    template <typename T>
    static inline AbstractFile* load(QDataStream& stream) {
        auto* file = new T;
        stream >> *file;
        return file;
    }

    AbstractFile();

    virtual ~AbstractFile();

    QString shortName() const;
    QString name() const;
    void setFileName(const QString& fileName);

    void addToScene() const;

    GiGroup* itemGroup(int type = -1) const;

    const mvector<GiGroup*>& itemGroups() const;

    Paths mergedPaths() const;
    Pathss groupedPaths() const;

    mvector<QString>& lines();
    const mvector<QString>& lines() const;
    const QString lines2() const;
    virtual mvector<const AbstrGraphicObject*> graphicObjects() const;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    const LayerTypes& displayedTypes() const;
    Side side() const;
    void setSide(Side side);

    virtual void initFrom(AbstractFile* file);
    virtual uint32_t type() const = 0;
    virtual QString loadErrorMessage() const = 0;
    virtual void createGi() = 0;
    virtual void setItemType([[maybe_unused]] int type);
    virtual int itemsType() const;

    virtual bool isVisible() const;
    virtual void setVisible(bool visible);
    const QColor& color() const;
    virtual void setColor(const QColor& color);

    virtual FileTree::Node* node() = 0;
    virtual QIcon icon() const;

    void setTransform([[maybe_unused]] const Transform& transform);
    const Transform& transform() const;

    int id() const;
    void setId(int id);

    bool userColor() const;
    void setUserColor(bool userColor);

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

// #include <QApplication>
#include <QFileInfo>
// #include <QModelIndex>

inline AbstractFile::AbstractFile()
    : itemGroups_ {new GiGroup} {
}

inline AbstractFile::~AbstractFile() { qDeleteAll(itemGroups_); }

inline QString AbstractFile::shortName() const { return QFileInfo(name_).fileName(); }

inline QString AbstractFile::name() const { return name_; }

inline void AbstractFile::setFileName(const QString& fileName) { name_ = fileName; }

inline void AbstractFile::addToScene() const {
    for (const auto var : itemGroups_) {
        if (var && var->size()) {
            var->addToScene();
            var->setZValue(-id_);
        }
    }
}

inline GiGroup* AbstractFile::itemGroup(int type) const {
    const int size(static_cast<int>(itemGroups_.size()));
    if (type == -1 && 0 <= itemsType_ && itemsType_ < size)
        return itemGroups_[itemsType_];
    else if (0 <= type && type < size)
        return itemGroups_[type];
    return itemGroups_.front();
}

inline const mvector<GiGroup*>& AbstractFile::itemGroups() const { return itemGroups_; }

inline Paths AbstractFile::mergedPaths() const { return mergedPaths_.size() ? mergedPaths_ : merge(); }

inline Pathss AbstractFile::groupedPaths() const { return groupedPaths_; }

inline mvector<QString>& AbstractFile::lines() { return lines_; }

inline const mvector<QString>& AbstractFile::lines() const { return lines_; }

inline const QString AbstractFile::lines2() const {
    QString rstr;
    for (auto&& str : lines_)
        rstr.append(str).append('\n');
    return rstr;
}

inline mvector<const AbstrGraphicObject*> AbstractFile::graphicObjects() const { return {}; }

inline void AbstractFile::initFrom(AbstractFile* file) {
    id_ = file->id();
    node_ = file->node();
    // node_->setId(&id_);
}

inline const LayerTypes& AbstractFile::displayedTypes() const { return layerTypes_; }

inline void AbstractFile::setItemType(int type) { }

inline int AbstractFile::itemsType() const { return itemsType_; }

inline Side AbstractFile::side() const { return side_; }

inline void AbstractFile::setSide(Side side) { side_ = side; }

inline bool AbstractFile::isVisible() const { return (visible_ = itemGroup()->isVisible()); }

inline void AbstractFile::setVisible(bool visible) { itemGroup()->setVisible(visible_ = visible); }

inline const QColor& AbstractFile::color() const { return color_; }

inline void AbstractFile::setColor(const QColor& color) { color_ = color; }

inline void AbstractFile::setTransform(const Transform& transform) {
    transform_ = transform;
    QTransform t {transform};
    for (auto* ig : itemGroups_)
        for (auto* gi : *ig)
            gi->setTransform(t);
}

inline const Transform& AbstractFile::transform() const { return transform_; }

inline int AbstractFile::id() const { return id_; }

inline void AbstractFile::setId(int id) { id_ = id; }

inline QIcon AbstractFile::icon() const { return {}; }

inline bool AbstractFile::userColor() const { return colorFlag_; }

inline void AbstractFile::setUserColor(bool userColor) { colorFlag_ = userColor; }

#define FileInterface_iid "ru.xray3d.XrSoft.GGEasy.AbstractFile"

Q_DECLARE_INTERFACE(AbstractFile, FileInterface_iid)
