/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include <QObject>
#include <app.h>
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
    [[nodiscard]] virtual Shapes::Shape* createShape(const QPointF& point) = 0;
    virtual bool addShapePoint(const QPointF& point) = 0;
    virtual void updateShape(const QPointF& point) = 0;
    virtual void finalizeShape() = 0;
    [[nodiscard]] virtual QJsonObject info() const = 0;
    [[nodiscard]] virtual QIcon icon() const = 0; //    virtual void addToDrillForm([[maybe_unused]] FileInterface* file, [[maybe_unused]] QComboBox* cbx) {};
    //    virtual void createMainMenu([[maybe_unused]] QMenu& menu, [[maybe_unused]] FileTreeView* tv)
    //    {
    //        menu.addAction(QIcon::fromTheme("document-close"), QObject::tr("&Close All Files"), [tv] {
    //            if (QMessageBox::question(tv, "", QObject::tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    //                tv->closeFiles();
    //        });
    //    };
    virtual void setupInterface(App* a) = 0;

signals:
    virtual void actionUncheck(bool = false) = 0;

    // slots:
    //    virtual FileInterface* parseFile(const QString& fileName, int type) = 0;

protected:
    App app;
    enum { IconSize = 24 };
};

#define ShapePlugin_iid "ru.xray3d.XrSoft.GGEasy.ShapePluginInterface"

Q_DECLARE_INTERFACE(ShapePluginInterface, ShapePlugin_iid)
