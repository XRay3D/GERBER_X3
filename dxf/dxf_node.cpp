// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "dxf_node.h"

#include "dxf_file.h"
#include "tables/dxf_layer.h"
#include "tables/dxf_layermodel.h"

#include "scene.h"
#include <QColorDialog>
#include <QIcon>
#include <QMenu>
#include <QtWidgets>
#include <filetree/treeview.h>
#include <forms/drillform/drillform.h>

#include "leakdetector.h"

namespace Dxf {

class Dialog : public QDialog {
    QVBoxLayout* verticalLayout;
    QPushButton* pushButtonColorize;
    QTableView* tableView;
    void setupUi(QDialog* dialog)
    {
        if (dialog->objectName().isEmpty())
            dialog->setObjectName(QString::fromUtf8("Dialog"));
        verticalLayout = new QVBoxLayout(dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(6, 6, 6, 6);

        pushButtonColorize = new QPushButton(dialog);
        pushButtonColorize->setObjectName(QString::fromUtf8("pushButtonColorize"));
        pushButtonColorize->setText(tr("Colorize"));
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
        , fl(fl = false)
    {
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
                //tableView->hideRow(row);
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
            const int count = colors.size();
            int ctr = 0;
            for (auto& [name, color] : colors) {
                const int k = static_cast<int>((count > 1) ? (200.0 / (count - 1)) * ctr++ : 0);
                auto layer = file->layers().at(name);
                layer->setColor(QColor::fromHsv(k, 255, 255));
                for (auto gi : *layer->itemGroup())
                    gi->changeColor();
            }
            tableView->reset();
        });

        setGeometry({ parent->mapToGlobal({}), parent->size() });
    }

    // QWidget interface
protected:
    void hideEvent(QHideEvent* event) override
    {
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
        , fl(fl = false)
    {
        setAlternatingRowColors(true);
        setAnimated(true);
        setUniformRowHeights(true);

        setWindowTitle("Section HEADER");
        setColumnCount(2);
        auto iPar = file->header().cbegin();
        QList<QTreeWidgetItem*> items;
        while (iPar != file->header().cend()) {
            items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(iPar->first)));
            {
                auto iVal = iPar->second.cbegin();
                while (iVal != iPar->second.cend()) {
                    items.last()->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr),
                        { QString::number(iVal->first), iVal->second.toString() }));
                    ++iVal;
                }
            }
            ++iPar;
        }
        insertTopLevelItems(0, items);
        setWindowFlag(Qt::WindowStaysOnTopHint, true);
        expandAll();
        setGeometry({ parent->mapToGlobal({}), parent->size() });

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
            i.save("vline.png", "PNG");
            // ├─
            p.drawLine(w >> 1, h >> 1, /**/ w, h >> 1);
            i.save("branch-more.png", "PNG");
            // └─
            i.fill(Qt::transparent);
            p.drawLine(w >> 1, /**/ 0, w >> 1, h >> 1);
            p.drawLine(w >> 1, h >> 1, /**/ w, h >> 1);
            i.save("branch-end.png", "PNG");
            QFile file(":/qtreeviewstylesheet/QTreeView.qss");
            file.open(QFile::ReadOnly);
            setStyleSheet(file.readAll());
            header()->setMinimumHeight(h);
        }
    }
    // QWidget interface
protected:
    void hideEvent(QHideEvent* event) override
    {
        event->accept();
        deleteLater();
    }
};

Node::Node(int id)
    : AbstractNode(id)
{
    File* f = static_cast<File*>(file());
    for (auto ig : f->itemGroups())
        ig->addToScene();
    //    for (auto& [name, layer] : f->layers()) {
    //        if (layer->itemGroup())
    //            append(new Dxf::NodeLayer(name, layer));
    //    }
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (static_cast<Column>(index.column())) {
    case Column::NameColorVisible:
        switch (role) {
        case Qt::CheckStateRole:
            file()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
    case Column::SideType:
        switch (role) {
        case Qt::EditRole:
            file()->setSide(static_cast<Side>(value.toBool()));
            return true;
        default:
            return false;
        }
    case Column::ItemsType:
        switch (role) {
        case Qt::EditRole:
            qDebug() << __FUNCTION__ << role << value;
            App::project()->file<File>(m_id)->setItemType(static_cast<ItemsType>(value.toInt()));
            return true;
        default:
            return false;
        }
    default:
        return false;
    }
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    switch (static_cast<Column>(index.column())) {
    case Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case Column::SideType:
        return itemFlag | Qt::ItemIsEditable;
    case Column::ItemsType:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const
{
    switch (static_cast<Column>(index.column())) {
    case Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
            return file()->shortName();
        case Qt::ToolTipRole:
            return file()->shortName() + "\n" + file()->name();
        case Qt::CheckStateRole:
            return file()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return QIcon::fromTheme("crosshairs");
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case Column::SideType:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return sideStrList[file()->side()];
        case Qt::EditRole:
            return static_cast<bool>(file()->side());
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case Column::ItemsType:
        switch (role) {
        case Qt::DisplayRole:
            return file()->displayedTypes().at(int(App::project()->file<File>(m_id)->itemsType())).actName;
        case Qt::ToolTipRole:
            return file()->displayedTypes().at(int(App::project()->file<File>(m_id)->itemsType())).actName;
        case Qt::EditRole:
            return file()->displayedTypes().at(int(App::project()->file<File>(m_id)->itemsType())).id;
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
    return QVariant();
}

void Node::menu(QMenu* menu, FileTreeView* tv) const
{
    menu->addAction(QIcon::fromTheme("hint"), QObject::tr("&Hide other"), tv, &FileTreeView::hideOther);
    menu->addAction(QIcon::fromTheme("document-close"), QObject::tr("&Close"), tv, &FileTreeView::closeFile);

    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("color-management"), QObject::tr("Colorize"), [this] {
        const int count = childCount();
        for (int row = 0; row < count; ++row) {
            const int k = static_cast<int>((count > 1) ? (200.0 / (count - 1)) * row : 0);
            NodeLayer* nl = reinterpret_cast<NodeLayer*>(child(row));
            nl->layer->setColor(QColor::fromHsv(k, 255, 255));
            for (auto gi : *nl->layer->itemGroup())
                gi->changeColor();
        }
    });

    menu->addSeparator();
    if (layer)
        menu->addAction(QIcon::fromTheme("layer-visible-on"), QObject::tr("&Layers"), [tv, this] {
            auto dialog = new Dialog(static_cast<File*>(file()), layer, tv);
            dialog->show();
        });
    if (header)
        menu->addAction(QIcon::fromTheme("/*document-close*/"), QObject::tr("Header"), [tv, this] {
            auto tw = new TreeWidget(static_cast<File*>(file()), header, tv);
            tw->show();
        });

    { // QActionGroup
        menu->addSeparator();

        auto group = new QActionGroup(menu);

        File* file = static_cast<File*>(this->file());

        auto action = menu->addAction(QObject::tr("&Solid"), [=](bool checked) {
            file->setItemType(static_cast<ItemsType>(checked * static_cast<int>(ItemsType::Normal)));
        });
        action->setCheckable(true);
        action->setChecked(file->itemsType() == ItemsType::Normal);
        action->setToolTip(QObject::tr("Normal"));
        action->setActionGroup(group);

        action = menu->addAction(QObject::tr("&Paths"), [=](bool checked) {
            file->setItemType(static_cast<ItemsType>(checked * static_cast<int>(ItemsType::Paths)));
        });
        action->setCheckable(true);
        action->setChecked(file->itemsType() == ItemsType::Paths);
        action->setToolTip(QObject::tr("Paths_")); //Displays only aperture paths of copper\nwithout width and without contacts."));
        action->setActionGroup(group);
    }
}

///////////////////////////////////
///// \brief Node::NodeLayer
///// \param id
/////
NodeLayer::NodeLayer(const QString& name, Layer* layer)
    : AbstractNode(-1, -1)
    , name(name)
    , layer(layer)
{
}

bool NodeLayer::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (static_cast<Column>(index.column())) {
    case Column::NameColorVisible:
        if (role == Qt::CheckStateRole)
            layer->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
        return true;
    case Column::ItemsType:
        if (role == Qt::EditRole)
            layer->setItemsType(value.toInt() ? ItemsType::Paths : ItemsType::Normal);
        return true;
    default:
        return false;
    }
}

Qt::ItemFlags NodeLayer::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren; //| Qt::ItemIsSelectable;
    switch (static_cast<Column>(index.column())) {
    case Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case Column::ItemsType:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant NodeLayer::data(const QModelIndex& index, int role) const
{
    switch (static_cast<Column>(index.column())) {
    case Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return name;
        case Qt::CheckStateRole:
            return layer->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return decoration(layer->color());
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case Column::ItemsType:
        switch (role) {
        case Qt::DisplayRole:
            return layer->file()->displayedTypes().at(int(layer->itemsType())).actName;
        case Qt::ToolTipRole:
            return layer->file()->displayedTypes().at(int(layer->itemsType())).actName;
        case Qt::EditRole:
            return layer->file()->displayedTypes().at(int(layer->itemsType())).id;
        case Qt::UserRole:
            return layer->file()->id();
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
    return QVariant();
}

void NodeLayer::menu(QMenu* menu, FileTreeView* tv) const
{
    menu->addAction(QIcon::fromTheme("color-management"), QObject::tr("Change color"), [tv, this] {
        QColorDialog cd(tv);
        cd.setCurrentColor(layer->color());
        if (cd.exec()) {
            layer->setColor(cd.currentColor());
            for (auto gi : *layer->itemGroup())
                gi->changeColor();
        }
    });
}
}
