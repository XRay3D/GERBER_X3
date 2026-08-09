/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_plugin.h"
#include "abstract_file.h"
#include "app.h"
#include "ft_model.h"
#include "gc_form.h"
#include "gc_highlighter.h"
#include "gc_node.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QToolBar>

namespace GCode {

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin{parent} { }

// Plugin::Plugin(QObject* parent)
// : AbstractFilePlugin{parent} {
// info_ = {
// {        u"Name"_s,                                                 u"GCode"_s},
// {     u"Version"_s,                                                   u"1.1"_s},
// {u"VendorAuthor"_s,                                u"X-Ray aka Bakiev Damir"_s},
// {        u"Info"_s, u"GCode is a static plugin always included with GGEasy."_s}
// };
// }

QAction* Plugin::addAction(QMenu* menu, QToolBar* toolbar) {
    auto action = toolbar->addAction(icon(), info()[u"Name"_s].toString());
    connect(action, &QAction::toggled, [=, this](bool checked) {
        if(checked && canToShow())
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
    menu.addAction(QIcon::fromTheme(u"edit-delete"_s), tr("&Delete All Toolpaths"), [tv] {
        if(QMessageBox::question(tv, {}, tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            tv->closeFiles();
    });
    menu.addAction(QIcon::fromTheme(u"document-save-all"_s), tr("&Save Selected Tool Paths..."),
        tv, &FileTree::View::saveSelectedGCodeFiles);
}

void Plugin::editFile(File* file) {
    auto* widget = createForm();
    // setDockWidget идемпотентен: если форма уже в доке, он выйдет сразу.
    emit setDockWidget(widget);
    static_cast<Form*>(widget)->editFile(file);
}

void Plugin::updateFileModel(AbstractFile* file) {
    // Замена УП на месте: initFrom перенёс на новый файл id и указатель на узел,
    // но сам узел всё ещё держит указатель на старый, уже удалённый объект.
    auto* node = static_cast<Node*>(file->node());
    node->setFile(file);
    App::fileModel().updateNode(node);
    // id прежний, объект новый: открытое окно просмотра держит текст старого.
    Dialog::programChanged(file->id(), file->lines2(), file->name());
}

// AbstractFileSettings* Plugin::createSettingsTab(QWidget* parent) {
// auto tab = new Tab{parent};
// tab->setWindowTitle(u"G-Code"_s);
// return tab;
//}

} // namespace GCode

#include "moc_gc_plugin.cpp"
