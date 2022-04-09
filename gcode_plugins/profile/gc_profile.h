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

#include "gc_creator.h"
#include "gc_odeplugininterface.h"

#include <QIcon>
#include <QJsonObject>
#include <QPixmap>

class ProfilePlugin : public QObject, public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "profile.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    ProfilePlugin(QObject* parent = nullptr)
        : QObject(parent) { }

    QObject* getObject() override { return this; }
    int type() const override { return GCode::Profile; }
    QJsonObject info() const override {
        return {
            { "Name", "Profile" },
            { "Version", "1.0" },
            { "VendorAuthor", "X-Ray aka Bakiev Damir" },
            { "Info", "Profile" },
        };
    }

    virtual QIcon icon() const override { return QPixmap { 16, 16 }; }

signals:
    void actionUncheck(bool = false) override;
};

namespace GCode {

class ProfileCreator : public Creator {
public:
    ProfileCreator();
    ~ProfileCreator() override = default;

private:
    void createProfile(const Tool& tool, const double depth);
    void trimmingOpenPaths(Paths& paths);

    IntPoint from;

    void cornerTrimming();
    void makeBridges();

    void reorder();
    void reduceDistance(IntPoint& from, Path& to);
    enum NodeType {
        ntAny,
        ntOpen,
        ntClosed
    };
    void polyTreeToPaths(PolyTree& polytree, Paths& rpaths);
    //    void addPolyNodeToPaths(PolyNode& polynode, NodeType nodetype, Paths& paths);
    //    void closedPathsFromPolyTree(PolyTree& polytree, Paths& paths);
    //    void openPathsFromPolyTree(const PolyTree& polytree, Paths& paths);

protected:
    void create() override; // Creator interface
    GCodeType type() override;
};

} // namespace GCode
