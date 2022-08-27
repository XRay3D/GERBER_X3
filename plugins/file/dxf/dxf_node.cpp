// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_node.h"

#include "dxf_file.h"
#include "dxf_sourcedialog.h"
#include "tables/dxf_layer.h"
#include "tables/dxf_layermodel.h"

#include "ft_view.h"
#include "scene.h"

#include <QColorDialog>
#include <QIcon>
#include <QMenu>
#include <QtWidgets>

namespace Dxf {

class Dialog : public QDialog {
    QVBoxLayout* verticalLayout;
    QPushButton* pushButtonColorize;
    QTableView* tableView;
    void setupUi(QDialog* dialog) {
        if (dialog->objectName().isEmpty())
            dialog->setObjectName(QString::fromUtf8("Dialog"));
        verticalLayout = new QVBoxLayout(dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(6, 6, 6, 6);

        pushButtonColorize = new QPushButton(dialog);
        pushButtonColorize->setObjectName(QString::fromUtf8("pushButtonColorize"));
        pushButtonColorize->setText(DxfObj::tr("Colorize"));
        pushButtonColorize->setIcon(QIcon::fromTheme("color-management"));
        verticalLayout->addWidget(pushButtonColorize);

        tableView = new QTableView(dialog);
        tableView->setObjectName(QString::fromUtf8("tableView"));
        verticalLayout->addWidget(tableView);

        QMetaObject::connectSlotsByName(dialog);
    }
    bool& fl;

public:
    ~Dialog() { fl = true; }
    Dialog(File* file, bool& fl, QWidget* parent = nullptr)
        : QDialog(parent)
        , fl(fl = false) {
        setupUi(this);

        setWindowTitle(file->shortName());

        tableView->setModel(new LayerModel(file->layers(), tableView));
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableView->horizontalHeader()->setSectionResizeMode(LayerModel::Type, QHeaderView::Stretch);
        tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableView->setItemDelegateForColumn(LayerModel::Type, new ItemsTypeDelegate(tableView));

        QStringList names(keys(file->layers()));
        std::map<QString, QColor> colors;
        for (int row = 0; row < names.size(); ++row) {
            if (file->layers().at(names[row])->isEmpty()) {
                // tableView->hideRow(row);
            } else {
                colors[names[row]];
            }
        }

        connect(tableView, &QTableView::doubleClicked, [names, file, this](const QModelIndex& index) {
            if (index.column() == 0) {
                QColorDialog cd(this);
                cd.setCurrentColor(file->layers().at(names[index.row()])->color());
                if (cd.exec())
                    tableView->model()->setData(index, cd.currentColor(), Qt::DecorationRole);
            }
        });

        connect(pushButtonColorize, &QPushButton::clicked, [colors, file, this] {
            const size_t count = colors.size();
            size_t ctr = 0;
            for (auto& [name, color] : colors) {
                const int k = static_cast<int>((count > 1) ? (200.0 / (count - 1)) * ctr++ : 0);
                auto layer = file->layers().at(name);
                layer->setColor(QColor::fromHsv(k, 255, 255));
                for (auto gi : *layer->itemGroup())
                    gi->changeColor();
            }
            tableView->reset();
        });
        setGeometry({parent->mapToGlobal(QPoint()), parent->size()});
    }

    // QWidget interface
protected:
    void hideEvent(QHideEvent* event) override {
        event->accept();
        deleteLater();
    }
};

class TreeWidget : public QTreeWidget {
    bool& fl;

public:
    ~TreeWidget() { fl = true; }
    TreeWidget(File* file, bool& fl, QWidget* parent = nullptr)
        : QTreeWidget(nullptr)
        , fl(fl = false) {
        setAlternatingRowColors(true);
        setAnimated(true);
        setUniformRowHeights(true);

        setWindowTitle(DxfObj::tr("Section HEADER"));
        setColumnCount(2);
        auto iPar = file->header().cbegin();
        QList<QTreeWidgetItem*> items;
        while (iPar != file->header().cend()) {
            items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(iPar->first)));
            {
                auto iVal = iPar->second.cbegin();
                while (iVal != iPar->second.cend()) {
                    items.last()->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),
                        {QString::number(iVal->first), iVal->second.toString()}));
                    ++iVal;
                }
            }
            ++iPar;
        }
        insertTopLevelItems(0, items);
        setWindowFlag(Qt::WindowStaysOnTopHint, true);
        expandAll();
        setGeometry({parent->mapToGlobal(QPoint()), parent->size()});

        {
            setIconSize(QSize(24, 24));
            const int w = indentation();
            const int h = rowHeight(model()->index(0, 0, QModelIndex()));
            QImage i(w, h, QImage::Format_ARGB32);
            QPainter p(&i);
            p.setPen(QColor(128, 128, 128));
            // │
            i.fill(Qt::transparent);
            p.drawLine(w >> 1, /**/ 0, w >> 1, /**/ h);
            i.save("settings/vline.png", "PNG");
            // ├─
            p.drawLine(w >> 1, h >> 1, /**/ w, h >> 1);
            i.save("settings/branch-more.png", "PNG");
            // └─
            i.fill(Qt::transparent);
            p.drawLine(w >> 1, /**/ 0, w >> 1, h >> 1);
            p.drawLine(w >> 1, h >> 1, /**/ w, h >> 1);
            i.save("settings/branch-end.png", "PNG");
            QFile file(":/qtreeviewstylesheet/QTreeView.qss");
            file.open(QFile::ReadOnly);
            setStyleSheet(file.readAll());
            header()->setMinimumHeight(h);
        }
    }
    // QWidget interface
protected:
    void hideEvent(QHideEvent* event) override {
        event->accept();
        deleteLater();
    }
};

Node::Node(File* file)
    : FileTree::Node(file->id(), FileTree::File)
    , file(file) {
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch (role) {
    case Qt::CheckStateRole:
        file->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
        emit App::fileModel()->dataChanged(childs.front()->index(index.column()), childs.back()->index(index.column()), {role});
        return true;
    case Qt::EditRole:
        switch (FileTree::Column(index.column())) {
        case FileTree::Column::Side:
            if (role == Qt::EditRole) {
                file->setSide(static_cast<Side>(value.toBool()));
                // emit App::fileModel()->dataChanged(childs.front()->index(index.column()), childs.back()->index(index.column()), { role });
                return true;
            }
        case FileTree::Column::ItemsType:
            if (role == Qt::EditRole) {
                file->setItemType(value.toInt());
                emit App::fileModel()->dataChanged(childs.front()->index(index.column()), childs.back()->index(index.column()), {role});
                return true;
            }
        default:
            break;
        }
        break;
    case FileTree::Select:
        for (auto ig : file->itemGroups())
            ig->setZValue((value.toBool() ? +(file->id() + 1) : -(file->id() + 1)) * 1000);
        return true;
    }
    return {};
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side:
        return itemFlag | Qt::ItemIsEditable;
    case FileTree::Column::ItemsType:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const {
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
            return file->shortName();
        case Qt::ToolTipRole:
            return file->shortName() + "\n" + file->name();
        case Qt::CheckStateRole:
            return file->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return QIcon::fromTheme("crosshairs");
        }
        break;
    case FileTree::Column::Side:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return sideStrList[file->side()];
        case Qt::EditRole:
            return static_cast<bool>(file->side());
        }
        break;
    case FileTree::Column::ItemsType:
        switch (role) {
        case Qt::DisplayRole:
            return file->displayedTypes().at(int(file->itemsType())).shortActName();
        case Qt::ToolTipRole:
            return file->displayedTypes().at(int(file->itemsType())).actToolTip;
        case Qt::EditRole:
            return file->displayedTypes().at(int(file->itemsType())).id;
        }
        break;
    default:
        break;
    }
    switch (role) {
    case FileTree::Id:
        return id_.get();
    default:
        return QVariant();
    }
    return QVariant();
}

void Node::menu(QMenu& menu, FileTree::View* tv) const {
    menu.addAction(QIcon::fromTheme("hint"), DxfObj::tr("&Hide other"), tv, &FileTree::View::hideOther);
    menu.addAction(QIcon(), DxfObj::tr("&Show source"), [tv, this] {
        auto dialog = new SourceDialog(id_, tv);
        dialog->exec();
        delete dialog;
    });
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("color-management"), DxfObj::tr("Colorize"), [this] {
        const int count = childCount();
        for (int row = 0; row < count; ++row) {
            const int k = static_cast<int>((count > 1) ? (200.0 / (count - 1)) * row : 0);
            NodeLayer* nl = reinterpret_cast<NodeLayer*>(child(row));
            nl->layer->setColor(QColor::fromHsv(k, 255, 255));
            for (auto gi : *nl->layer->itemGroup())
                gi->changeColor();
        }
    });

    menu.addSeparator();
    if (layer)
        menu.addAction(QIcon::fromTheme("layer-visible-on"), DxfObj::tr("&Layers"), [tv, this] {
            auto dialog = new Dialog(static_cast<File*>(file), layer, tv);
            dialog->show();
        });
    if (header)
        menu.addAction(QIcon::fromTheme("/*document-close*/"), DxfObj::tr("Header"), [tv, this] {
            auto tw = new TreeWidget(static_cast<File*>(file), header, tv);
            tw->show();
        });

    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("document-close"), DxfObj::tr("&Close"), tv, &FileTree::View::closeFile);
}

///////////////////////////////////
///// \brief Node::NodeLayer
///// \param id
NodeLayer::NodeLayer(const QString& name, Layer* layer)
    : FileTree::Node(layer->file()->id(), FileTree::SubFile)
    , name(name)
    , layer(layer) {
}

bool NodeLayer::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        if (role == Qt::CheckStateRole) {
            bool visible = value.value<Qt::CheckState>() == Qt::Checked;
            layer->setVisible(visible);
            layer->file()->layersVisible_[name] = visible;
            if (visible) {
                layer->file()->visible_ = visible;
                emit App::fileModel()->dataChanged(parent_->index(index.column()), parent_->index(index.column()), {role});
            }
        }
        return true;
    case FileTree::Column::ItemsType:
        if (role == Qt::EditRole)
            layer->setItemsType(static_cast<ItemsType>(value.toInt()));
        return true;
    default:
        return false;
    }
}

Qt::ItemFlags NodeLayer::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren; //| Qt::ItemIsSelectable;
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::ItemsType:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant NodeLayer::data(const QModelIndex& index, int role) const {
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return name;
        case Qt::CheckStateRole:
            return layer->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return decoration(layer->color());
        case FileTree::Id:
            return id_.get();
        default:
            return QVariant();
        }
    case FileTree::Column::ItemsType: {
        auto file(layer->file());
        int type(static_cast<int>(layer->itemsType()));
        switch (role) {
        case Qt::DisplayRole:
            return file->displayedTypes().at(type).shortActName();
        case Qt::ToolTipRole:
            return file->displayedTypes().at(type).actToolTip;
        case Qt::EditRole:
            return file->displayedTypes().at(type).id;
        case FileTree::Id:
            return file->id();
        default:
            return QVariant();
        }
    }
    default:
        return QVariant();
    }
    return QVariant();
}

void NodeLayer::menu(QMenu& menu, FileTree::View* tv) const {
    menu.addAction(QIcon::fromTheme("color-management"), DxfObj::tr("Change color"), [tv, this] {
        QColorDialog cd(tv);
        cd.setCurrentColor(layer->color());
        if (cd.exec()) {
            layer->setColor(cd.currentColor());
            for (auto gi : *layer->itemGroup())
                gi->changeColor();
        }
    });
}

} // namespace Dxf
