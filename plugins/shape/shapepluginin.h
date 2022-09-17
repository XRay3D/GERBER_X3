/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "ft_view.h"
#include "shape.h"

#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <atomic>
namespace Shapes {
class Node;
class Shape;

class Plugin : public QObject {
    Q_OBJECT

    //    std::atomic<Shape*> item {};
    Shape* item {};

public:
    explicit Plugin();
    virtual ~Plugin() = default;

    void addPoint(const QPointF& point);
    void updPoint(const QPointF& point);
    void finalizeShape();

    virtual QIcon icon() const = 0;
    [[nodiscard]] virtual Shape* createShape(const QPointF& point = {}) const = 0;
    virtual int type() const = 0;

    void createMainMenu(QMenu& menu, FileTree::View* tv);

    QString folderName() const;
    const QJsonObject& info() const;
    void setInfo(const QJsonObject& info);

signals:
    void actionUncheck(bool = false);

protected:
    QJsonObject info_;
    enum { IconSize = 24 };
};

} // namespace Shapes

#define ShapePlugin_iid "ru.xray3d.XrSoft.GGEasy.Plugin"

Q_DECLARE_INTERFACE(Shapes::Plugin, ShapePlugin_iid)
