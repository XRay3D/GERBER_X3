// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_plugin.h"
#include "app.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QToolBar>

GCodePlugin::GCodePlugin(QObject* parent)
    : QObject {parent} {
    App app;
}

bool GCodePlugin::canToShow() const { return true; }

QAction* GCodePlugin::addAction(QMenu* menu, QToolBar* toolbar) {
    auto action = toolbar->addAction(icon(), info()["Name"].toString());
    connect(action, &QAction::toggled, [=, this](bool checked) {
        if (checked && canToShow())
            emit setDockWidget(createForm());
        else
            action->setChecked(false);
    });
    action->setShortcut(keySequence());
    menu->addAction(action);
    return action;
}

const QJsonObject& GCodePlugin::info() const { return info_; }

void GCodePlugin::setInfo(const QJsonObject& info) { info_ = info; }

#include "moc_gc_plugin.cpp"
