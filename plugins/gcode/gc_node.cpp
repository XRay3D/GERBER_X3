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
#include "gc_node.h"
#include "abstract_file.h"
#include "gc_file.h"
#include "gc_programdialog.h"
#include "project.h"

#include <QFileInfo>
#include <QIcon>
#include <QMenu>

#include "ft_view.h"

namespace GCode {

Node::Node(AbstractFile* file)
    : FileTree::Node(FileTree::File)
    , file(file) {
}

Node::~Node() {
    // Файла больше нет -- смотреть в окне просмотра нечего.
    Dialog::programClosed(file->id());
    App::project().deleteFile(file->id());
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role) {

    switch(index.column()) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::CheckStateRole:
            file->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        case Qt::EditRole: {
            // Переименование по F2 меняет имя ПРОГРАММЫ, а отображаемое имя
            // пересобирается из него так же, как это делает Form::fileHandler.
            // Иначе programName_ разъехался бы с меткой в дереве, и поиск
            // коллизий начал бы срабатывать не на то.
            auto* gcFile = static_cast<File*>(file);
            gcFile->setProgramName(value.toString());
            // Params::tool() индексирует tools без проверки: у УП без инструмента
            // (отладочные файлы) суффикса просто не будет.
            const auto& params = gcFile->params();
            gcFile->setFileName(params.tools.empty()
                    ? gcFile->programName()
                    : gcFile->programName() + u'_' + params.tool().nameEnc());
            return true;
        }
        default:;
        }
        [[fallthrough]]; // роль FileTree::Select обрабатывается ниже, в default
    case FileTree::Column::Side:
        switch(role) {
        case Qt::EditRole: {
            const auto newSide = static_cast<Side>(value.toBool());
            if(newSide == file->side()) return true; // холостой выбор той же стороны
            file->setSide(newSide);
            static_cast<File*>(file)->regenerate(); // lines_ устарели при смене стороны платы
            return true;
        }
        default:;
        }
        [[fallthrough]];
    default:
        if(role == FileTree::Select) {
            file->itemGroup()->setZValue((value.toBool() ? +(file->id() + 1) : -(file->id() + 1)) * 1000);
            return true;
        }
        return false;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const {
    switch(index.column()) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole:
            // if (file->shortName().endsWith(App::gcSettings().fileExtension()))
            // return file->shortName();
            // else
            return {
                file->shortName() + QStringList{u"_TS"_s, u"_BS"_s}
                  [file->side()]
            };
        case Qt::EditRole      : return file->shortName();
        case Qt::ToolTipRole   : return {file->shortName() + u'\n' + file->name()};
        case Qt::CheckStateRole: return file->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole: return file->icon();
        case FileTree::Id      : return id();
        default                : return {};
        }
    case FileTree::Column::Side:
        switch(role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: return sideStrList[file->side()];
        case Qt::EditRole   : return static_cast<bool>(file->side());
        default             : return {};
        }
    default: return {};
    }
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable /*| Qt::ItemIsDragEnabled*/;
    switch(index.column()) {
    case FileTree::Column::NameColorVisible:
        // if (file->shortName().endsWith(App::gcSettings().fileExtension()))
        // return itemFlag | Qt::ItemIsUserCheckable;
        return itemFlag | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    case FileTree::Column::Side: {
        // if (file->shortName().endsWith(App::gcSettings().fileExtension()))
        // return itemFlag;
        return itemFlag | Qt::ItemIsEditable;
    }
    default: return itemFlag;
    }
}

void Node::menu(QMenu& menu, FileTree::View* tv) {
    menu.addAction(QIcon::fromTheme(u"document-save"_s), QObject::tr("&Save Toolpath"), [tv, this] {
        emit tv->saveGCodeFile(id());
    });
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme(u"hint"_s), QObject::tr("&Hide other"),
        tv, &FileTree::View::hideOther);
    // Реестр окон держит сам Dialog: одно окно на файл, оно же обновляется при
    // перегенерации и закрывается при удалении.
    menu.addAction(QIcon(), QObject::tr("&Show source"), [tv, this] {
        Dialog::showFor(id(), file->lines2(), file->name(), tv);
    });
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme(u"edit-delete"_s), QObject::tr("&Delete Toolpath"), tv, &FileTree::View::closeFile);
}

bool Node::doubleClicked(FileTree::View* tv) {
    emit tv->editGCodeFile(id());
    return true;
}

int Node::id() const { return file->id(); }

} // namespace GCode
