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

#include "ft_view.h"
#include "plugindata.h"
#include "shape.h"

#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <atomic>
namespace Shapes {
class Node;
class AbstractShape;

class Plugin : public QObject, public PluginData {
    Q_OBJECT

    //    std::atomic<AbstractShape*> item {};
    AbstractShape* item {};

public:
    explicit Plugin();
    virtual ~Plugin() = default;

    void addPoint(const QPointF& point);
    void updPoint(const QPointF& point);
    void finalizeShape();

    virtual QIcon icon() const = 0;
    [[nodiscard]] virtual AbstractShape* createShape(const QPointF& point = {}) const = 0;
    virtual uint32_t type() const = 0;

    void createMainMenu(QMenu& menu, FileTree::View* tv);

    QString folderName() const;

signals:
    void actionUncheck(bool = false);

protected:
    enum { IconSize = 24 };
};

} // namespace Shapes

#define ShapePlugin_iid "ru.xray3d.XrSoft.GGEasy.Plugin"

Q_DECLARE_INTERFACE(Shapes::Plugin, ShapePlugin_iid)
