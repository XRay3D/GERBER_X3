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

#include <QMenu>
#include <QMessageBox>
#include <QObject>

#include <app.h>
#include <ft_view.h>
#include <project.h>
#include <shape.h>

class ShapePluginInterface {
    static inline ShapePluginInterface* sp = nullptr;
    static inline Shapes::Shape* item;

public:
    static void addShapePoint_(const QPointF& point)
    {
        if (sp) {
            qDebug() << __FUNCTION__ << sp << sp;
            if (!item) {
                App::project()->addShape(item = sp->createShape(point));
            } else if (!sp->addShapePoint(point))
                finalizeShape_();
        }
    }
    static void updateShape_(const QPointF& point)
    {
        if (sp && item)
            sp->updateShape(point);
    }
    static void finalizeShape_()
    {
        qDebug() << __FUNCTION__;
        if (item)
            item->setSelected(true);
        if (sp)
            sp->finalizeShape();
        item = nullptr;
        sp = nullptr;
    }

    static void setShapePI(ShapePluginInterface* sp_) { sp = sp_; }

    explicit ShapePluginInterface() = default;
    virtual ~ShapePluginInterface() = default;
    virtual QObject* getObject() = 0;
    virtual int type() const = 0;
    [[nodiscard]] virtual Shapes::Shape* createShape() = 0;
    [[nodiscard]] virtual Shapes::Shape* createShape(const QPointF& point) = 0;
    virtual bool addShapePoint(const QPointF& point) = 0;
    virtual void updateShape(const QPointF& point) = 0;
    virtual void finalizeShape() = 0;
    [[nodiscard]] virtual QJsonObject info() const = 0;
    [[nodiscard]] virtual QIcon icon() const = 0;
    //    virtual void addToDrillForm([[maybe_unused]] FileInterface* file, [[maybe_unused]] QComboBox* cbx) {};
    void createMainMenu(QMenu& menu, FileTree::View* tv)
    {
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

#define ShapePlugin_iid "ru.xray3d.XrSoft.GGEasy.ShapePluginInterface"

Q_DECLARE_INTERFACE(ShapePluginInterface, ShapePlugin_iid)
