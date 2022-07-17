#include "gc_plugin.h"
#include "app.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QToolBar>

GCodePlugin::GCodePlugin(QObject* parent)
    : QObject { parent } {
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
