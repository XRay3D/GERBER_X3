/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "topor_node.h"

#include "app.h"
#include "ft_view.h"
#include "project.h"
#include "tables/topor_componentmodel.h"
#include "tables/topor_layer.h"
#include "tables/topor_layermodel.h"
#include "topor_file.h"

#include <QColorDialog>
#include <QHeaderView>
#include <QMenu>
#include <QTableView>
#include <QVBoxLayout>
#include <QtWidgets>

namespace TopoR {

class LayersDialog : public QDialog {
    QTableView* tableView;

public:
    LayersDialog(File* file, QWidget* parent = nullptr)
        : QDialog{parent} {
        setWindowTitle(file->shortName());

        auto layout = new QVBoxLayout{this};
        auto pushButtonColorize = new QPushButton{tr("Colorize"), this};
        pushButtonColorize->setIcon(QIcon::fromTheme(u"color-management"_s));
        layout->addWidget(pushButtonColorize);

        tableView = new QTableView{this};
        layout->addWidget(tableView);

        tableView->setModel(new LayerModel{file->layers(), tableView});
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableView->horizontalHeader()->setSectionResizeMode(LayerModel::Type, QHeaderView::Stretch);
        tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableView->setItemDelegateForColumn(LayerModel::Type, new ItemsTypeDelegate{tableView});

        connect(tableView, &QTableView::doubleClicked, [file, this](const QModelIndex& index) {
            if(index.column() == 0) {
                QColorDialog cd{this};
                cd.setCurrentColor(file->layers()[index.row()]->color());
                if(cd.exec())
                    tableView->model()->setData(index, cd.currentColor(), Qt::DecorationRole);
            }
        });

        connect(pushButtonColorize, &QPushButton::clicked, [file, this] {
            const auto& layers = file->layers();
            const size_t count = layers.size();
            for(size_t ctr{}; Layer* layer: layers) {
                const int k = static_cast<int>((count > 1) ? (200.0 / (count - 1)) * ctr++ : 0);
                layer->setColor(QColor::fromHsv(k, 255, 255));
            }
            tableView->reset();
        });

        resize(500, 400);
    }
};

class ComponentsDialog : public QDialog {
public:
    ComponentsDialog(File* file, QWidget* parent = nullptr)
        : QDialog{parent} {
        setWindowTitle(file->shortName() + u" - "_s + tr("Components"));
        auto layout = new QVBoxLayout{this};
        auto tableView = new QTableView{this};
        layout->addWidget(tableView);
        tableView->setModel(new ComponentModel{file->components(), tableView});
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableView->verticalHeader()->setVisible(false);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->setSortingEnabled(true);
        resize(600, 400);
    }
};

Node::Node(File* file)
    : FileTree::Node(FileTree::File)
    , file(file) {
}

Node::~Node() { App::project().deleteFile(file->id()); }

bool Node::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch(role) {
    case Qt::CheckStateRole:
        file->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
        if(childs.empty()) return true;
        emit App::fileModel().dataChanged(
            childs.front()->index(index.column()),
            childs.back()->index(index.column()), {role});
        return true;
    case Qt::EditRole:
        switch(FileTree::Column(index.column())) {
        case FileTree::Column::Side:
            file->setSide(static_cast<Side>(value.toBool()));
            return true;
        case FileTree::Column::ItemsType:
            file->setItemType(value.toInt());
            if(childs.empty()) return {};
            emit App::fileModel().dataChanged(
                childs.front()->index(index.column()),
                childs.back()->index(index.column()), {role});
            return true;
        default: break;
        }
        break;
    case FileTree::Select:
        for(auto ig: file->itemGroups())
            ig->setZValue((value.toBool() ? +(file->id() + 1) : -(file->id() + 1)) * 1000);
        return true;
    }
    return {};
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible: return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side            : return itemFlag | Qt::ItemIsEditable;
    case FileTree::Column::ItemsType       : return itemFlag | Qt::ItemIsEditable;
    default                                : return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole   : return file->shortName();
        case Qt::ToolTipRole   : return QVariant{file->shortName() + u'\n' + file->name()};
        case Qt::CheckStateRole: return file->isVisible() ? Qt::Checked : Qt::Unchecked;
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
    case FileTree::Column::ItemsType:
        switch(role) {
        case Qt::DisplayRole: return file->displayedTypes().at(file->itemsType()).shortActName();
        case Qt::ToolTipRole: return file->displayedTypes().at(file->itemsType()).actToolTip;
        case Qt::EditRole   : return file->displayedTypes().at(file->itemsType()).id;
        default             : return {};
        }
    default: return {};
    }
}

void Node::menu(QMenu& menu, FileTree::View* tv) {
    menu.addAction(QIcon::fromTheme(u"hint"_s), TopoRObj::tr("&Hide other"), tv, &FileTree::View::hideOther);
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme(u"color-management"_s), TopoRObj::tr("Colorize"), [this] {
        const int count = childCount();
        for(int row{}; row < count; ++row) {
            const int k = static_cast<int>((count > 1) ? (200.0 / (count - 1)) * row : 0);
            static_cast<NodeLayer*>(child(row))->layer->setColor(QColor::fromHsv(k, 255, 255));
        }
    });
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme(u"layer-visible-on"_s), TopoRObj::tr("&Layers"), [tv, this] {
        auto dialog = new LayersDialog{file, tv};
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    });
    if(!file->components().empty())
        menu.addAction(QIcon::fromTheme(u"view-list-details"_s), TopoRObj::tr("&Components"), [tv, this] {
            auto dialog = new ComponentsDialog{file, tv};
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->show();
        });
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme(u"document-close"_s), TopoRObj::tr("&Close"), tv, &FileTree::View::closeFile);
}

int32_t Node::id() const { return file->id(); }

NodeLayer::NodeLayer(const QString& name, Layer* layer)
    : FileTree::Node(FileTree::SubFile)
    , name(name)
    , layer(layer) {
}

bool NodeLayer::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        if(role == Qt::CheckStateRole) {
            bool visible = value.value<Qt::CheckState>() == Qt::Checked;
            layer->setVisible(visible);
            layer->file()->layersVisible_[name] = visible;
            if(visible) {
                layer->file()->setVisible(true);
                emit App::fileModel().dataChanged(parent_->index(index.column()), parent_->index(index.column()), {role});
            }
        }
        return true;
    case FileTree::Column::ItemsType:
        if(role == Qt::EditRole)
            layer->setItemsType(static_cast<ItemsType>(value.toInt()));
        return true;
    default: return false;
    }
}

Qt::ItemFlags NodeLayer::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible: return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::ItemsType       : return itemFlag | Qt::ItemIsEditable;
    default                                : return itemFlag;
    }
}

QVariant NodeLayer::data(const QModelIndex& index, int role) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole   : return name;
        case Qt::CheckStateRole: return layer->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole: return decoration(layer->color());
        case FileTree::Id      : return id();
        default                : return {};
        }
    case FileTree::Column::ItemsType: {
        File* file = layer->file();
        int type = static_cast<int>(layer->itemsType());
        switch(role) {
        case Qt::DisplayRole: return file->displayedTypes().at(type).shortActName();
        case Qt::ToolTipRole: return file->displayedTypes().at(type).actToolTip;
        case Qt::EditRole   : return file->displayedTypes().at(type).id;
        case FileTree::Id   : return file->id();
        default             : return {};
        }
    }
    default: return {};
    }
}

void NodeLayer::menu(QMenu& menu, FileTree::View* /*tv*/) {
    menu.addAction(QIcon::fromTheme(u"color-management"_s), TopoRObj::tr("Change color"), [this] {
        QColorDialog cd;
        cd.setCurrentColor(layer->color());
        if(cd.exec()) layer->setColor(cd.currentColor());
    });
}

int32_t NodeLayer::id() const { return layer->file()->id(); }

} // namespace TopoR
