// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "gc_plugin.h"
#include "app.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QToolBar>

namespace GCode {

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin {parent} {
    App app;
}

// Plugin::Plugin(QObject* parent)
//     : AbstractFilePlugin(parent) {
//     info_ = {
//         {        "Name",                                                 "GCode"},
//         {     "Version",                                                   "1.1"},
//         {"VendorAuthor",                                "X-Ray aka Bakiev Damir"},
//         {        "Info", "GCode is a static plugin always included with GGEasy."}
//     };
// }

QAction* Plugin::addAction(QMenu* menu, QToolBar* toolbar) {
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
///////////////////
void Plugin::createMainMenu(QMenu& menu, FileTree::View* tv) {
    menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete All Toolpaths"), [tv] {
        if (QMessageBox::question(tv, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            tv->closeFiles();
    });
    menu.addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."),
        tv, &FileTree::View::saveSelectedGCodeFiles);
}

AbstractFileSettings* Plugin::createSettingsTab(QWidget* parent) {

    auto tab = new Tab(parent);
    tab->setWindowTitle("G-Code");
    return tab;
}

} // namespace GCode

#include "moc_gc_plugin.cpp"
