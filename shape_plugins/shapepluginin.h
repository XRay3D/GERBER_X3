/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once

#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <app.h>
#include <ft_view.h>
#include <graphicsitem.h>
#include <project.h>

namespace Shapes {
class Node;
}

class ShapeInterface : public GraphicsItem {
    friend QDataStream& operator<<(QDataStream& stream, const ShapeInterface& shape) {
        stream << shape.type();
        stream << shape.m_giId;
        stream << shape.isVisible();
        shape.write_(stream);
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, ShapeInterface& shape) {
        stream >> shape.m_giId;
        bool visible;
        stream >> visible;
        shape.setZValue(shape.m_giId);
        shape.setVisible(visible);
        shape.setToolTip(QString::number(shape.m_giId));
        shape.read_(stream);
        return stream;
    }

public:
    ShapeInterface()
        : GraphicsItem(nullptr) {
    }
    virtual Shapes::Node* node() const = 0;

protected:
    virtual void write_([[maybe_unused]] QDataStream& stream) const = 0;
    virtual void read_([[maybe_unused]] QDataStream& stream) = 0;
};

class ShapePlugin {
    static inline ShapePlugin* sp = nullptr;
    static inline ShapeInterface* item;

public:
    static void addShapePoint_(const QPointF& point) {
        if (sp) {
            qDebug() << sp << sp;
            if (!item) {
                App::project()->addShape(item = sp->createShape(point));
            } else if (!sp->addShapePoint(point))
                finalizeShape_();
        }
    }
    static void updateShape_(const QPointF& point) {
        if (sp && item)
            sp->updateShape(point);
    }
    static void finalizeShape_() {
        qDebug(__FUNCTION__);
        if (item)
            item->setSelected(true);
        if (sp)
            sp->finalizeShape();
        item = nullptr;
        sp = nullptr;
    }

    static void setShapePI(ShapePlugin* sp_) { sp = sp_; }

    explicit ShapePlugin() = default;
    virtual ~ShapePlugin() = default;
    virtual QObject* toObj() = 0;
    virtual int type() const = 0;
    [[nodiscard]] virtual ShapeInterface* createShape() = 0;
    [[nodiscard]] virtual ShapeInterface* createShape(const QPointF& point) = 0;
    virtual bool addShapePoint(const QPointF& point) = 0;
    virtual void updateShape(const QPointF& point) = 0;
    virtual void finalizeShape() = 0;
    [[nodiscard]] virtual QJsonObject info() const = 0;
    [[nodiscard]] virtual QIcon icon() const = 0;
    //    virtual void addToDrillForm([[maybe_unused]] FileInterface* file, [[maybe_unused]] QComboBox* cbx) {};
    void createMainMenu(QMenu& menu, FileTree::View* tv) {
        menu.addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete All Shapes"), [tv] {
            if (QMessageBox::question(tv, "", QObject::tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                tv->closeFiles();
        });
    };
    QString folderName() { return QObject::tr("Shapes"); };

signals:
    virtual void actionUncheck(bool = false) = 0;

protected:
    App app;
    enum { IconSize = 24 };
};

#define ShapePlugin_iid "ru.xray3d.XrSoft.GGEasy.ShapePlugin"

Q_DECLARE_INTERFACE(ShapePlugin, ShapePlugin_iid)
