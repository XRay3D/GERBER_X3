// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_node.h"

#include "dxf_file.h"
#include "dxf_sourcedialog.h"
#include "tables/dxf_layer.h"
#include "tables/dxf_layermodel.h"

#include "scene.h"
#include "treeview.h"

#include <QColorDialog>
#include <QIcon>
#include <QMenu>
#include <QtWidgets>

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
        setGeometry({ parent->mapToGlobal(QPoint()), parent->size() });
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
                        { QString::number(iVal->first), iVal->second.toString() }));
                    ++iVal;
                }
            }
            ++iPar;
        }
        insertTopLevelItems(0, items);
        setWindowFlag(Qt::WindowStaysOnTopHint, true);
        expandAll();
        setGeometry({ parent->mapToGlobal(QPoint()), parent->size() });

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
    void hideEvent(QHideEvent* event) override
    {
        event->accept();
        deleteLater();
    }
};

File* Node::dxfFile() const { return static_cast<File*>(file()); }

Node::Node(int id)
    : NodeInterface(id)
{
    file()->addToScene();
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (Column(index.column())) {
    case Column::NameColorVisible:
        switch (role) {
        case Qt::CheckStateRole:
            file()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            emit App::fileModel()->dataChanged(childItems.first()->index(index.column()), childItems.last()->index(index.column()), { role });
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
            dxfFile()->setItemType(value.toInt());
            emit App::fileModel()->dataChanged(childItems.first()->index(index.column()), childItems.last()->index(index.column()), { role });
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
    switch (Column(index.column())) {
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
    switch (Column(index.column())) {
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
            return file()->displayedTypes().at(int(dxfFile()->itemsType())).shortActName();
        case Qt::ToolTipRole:
            return file()->displayedTypes().at(int(dxfFile()->itemsType())).actToolTip;
        case Qt::EditRole:
            return file()->displayedTypes().at(int(dxfFile()->itemsType())).id;
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

void Node::menu(QMenu& menu, FileTreeView* tv) const
{
    menu.addAction(QIcon::fromTheme("hint"), DxfObj::tr("&Hide other"), tv, &FileTreeView::hideOther);
    menu.addAction(QIcon(), DxfObj::tr("&Show source"), [tv, this] {
        auto dialog = new SourceDialog(m_id, tv);
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
            auto dialog = new Dialog(static_cast<File*>(file()), layer, tv);
            dialog->show();
        });
    if (header)
        menu.addAction(QIcon::fromTheme("/*document-close*/"), DxfObj::tr("Header"), [tv, this] {
            auto tw = new TreeWidget(static_cast<File*>(file()), header, tv);
            tw->show();
        });

    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("document-close"), DxfObj::tr("&Close"), tv, &FileTreeView::closeFile);
}

///////////////////////////////////
///// \brief Node::NodeLayer
///// \param id
/////
NodeLayer::NodeLayer(const QString& name, Layer* layer)
    : NodeInterface(-1, -1)
    , name(name)
    , layer(layer)
{
}

bool NodeLayer::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (Column(index.column())) {
    case Column::NameColorVisible:
        if (role == Qt::CheckStateRole) {
            bool visible = value.value<Qt::CheckState>() == Qt::Checked;
            layer->setVisible(visible);
            layer->file()->m_layersVisible[name] = visible;
            if (visible) {
                layer->file()->m_visible = visible;
                emit App::fileModel()->dataChanged(m_parentItem->index(index.column()), m_parentItem->index(index.column()), { role });
            }
        }
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
    switch (Column(index.column())) {
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
    switch (Column(index.column())) {
    case Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return name;
        case Qt::CheckStateRole:
            return layer->isVisible() ? Qt::Checked : Qt::Unchecked;
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
            return layer->file()->displayedTypes().at(int(layer->itemsType())).shortActName();
        case Qt::ToolTipRole:
            return layer->file()->displayedTypes().at(int(layer->itemsType())).actToolTip;
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

void NodeLayer::menu(QMenu& menu, FileTreeView* tv) const
{
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
}
