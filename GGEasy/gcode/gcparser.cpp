/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcparser.h"

#include "gcfile.h"
#include "gcnode.h"
#include "interfaces/file.h"
#include "treeview.h"
#include <QMenu>
#include <QMessageBox>

namespace GCode {

Parser::Parser(QObject* parent)
    : QObject(parent)
{
}

bool Parser::thisIsIt(const QString& /*fileName*/) { return false; }

QObject* Parser::getObject() { return this; }

int Parser::type() const { return int(FileType::GCode); }

NodeInterface* Parser::createNode(FileInterface* file) { return new Node(file->id()); }

std::shared_ptr<FileInterface> Parser::createFile() { return std::make_shared<File>(); }

void Parser::setupInterface(App*, AppSettings *s) { }

void Parser::createMainMenu(QMenu& menu, FileTreeView* tv)
{
    menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete All Toolpaths"), [tv] {
        if (QMessageBox::question(tv, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            tv->closeFiles();
    });
    menu.addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."),
        tv, &FileTreeView::saveSelectedGCodeFiles);
}

FileInterface* Parser::parseFile(const QString& /*fileName*/, int /*type*/) { return nullptr; }

}
