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
#include <QPainter>
#include <QtWidgets>
#include <filetree/treeview.h>
#include <forms/drillform/drillform.h>

#include "leakdetector.h"

namespace Dxf {

class Dialog : public QDialog {
    QVBoxLayout* verticalLayout;
    QTableView* tableView;
    void setupUi(QDialog* dialog)
    {
        if (dialog->objectName().isEmpty())
            dialog->setObjectName(QString::fromUtf8("Dialog"));
        verticalLayout = new QVBoxLayout(dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(6, 6, 6, 6);
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
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableView->setItemDelegateForColumn(LayerModel::Type, new ItemsTypeDelegate(tableView));

        connect(tableView, &QTableView::doubleClicked, [this](const QModelIndex& index) {
            if (index.column() == 0) {
                QColorDialog cd(this);
                if (cd.exec())
                    tableView->model()->setData(index, cd.currentColor(), Qt::DecorationRole);
            }
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
            setIconSize(QSize(22, 22));
            int w = indentation();
            int h = rowHeight(model()->index(0, 0, QModelIndex()));
            QImage i(w, h, QImage::Format_ARGB32);
            i.fill(Qt::transparent);
            for (int y = 0; y < h; ++y)
                i.setPixelColor(w / 2, y, QColor(128, 128, 128));
            i.save("vline.png", "PNG");

            for (int x = w / 2; x < w; ++x)
                i.setPixelColor(x, h / 2, QColor(128, 128, 128));
            i.save("branch-more.png", "PNG");

            i.fill(Qt::transparent);
            for (int y = 0; y < h / 2; ++y)
                i.setPixelColor(w / 2, y, QColor(128, 128, 128));
            for (int x = w / 2; x < w; ++x)
                i.setPixelColor(x, h / 2, QColor(128, 128, 128));
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
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case Name_:
        switch (role) {
        case Qt::CheckStateRole:
            file()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
    case Layer_:
        switch (role) {
        case Qt::EditRole:
            file()->setSide(static_cast<Side>(value.toBool()));
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
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled /*| Qt::ItemNeverHasChildren*/ | Qt::ItemIsSelectable;
    switch (index.column()) {
    case Name_:
        return itemFlag | Qt::ItemIsUserCheckable;
    case Layer_:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const
{
    if (file())
        switch (index.column()) {
        case Name_:
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
        case Layer_:
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
        default:
            return QVariant();
        }
    return QVariant();
}

void Node::menu(QMenu* menu, TreeView* tv) const
{
    menu->addAction(QIcon::fromTheme("hint"), QObject::tr("&Hide other"), tv, &TreeView::hideOther);
    menu->addAction(QIcon::fromTheme("document-close"), QObject::tr("&Close"), tv, &TreeView::closeFile);
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
//NodeLayer::NodeLayer(const QString& name, Layer* layer)
//    : AbstractNode(-1, -1)
//    , name(name)
//    , layer(layer)
//{
//    //App::scene()->addItem(layer->gig);
//}

//bool NodeLayer::setData(const QModelIndex& index, const QVariant& value, int role)
//{
//    switch (index.column()) {
//    case Name_:
//        switch (role) {
//            //        case Qt::CheckStateRole:
//            //            layer->gig->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
//            //            return true;
//        default:
//            return false;
//        }
//    case Layer_:
//        //        switch (role) {
//        //        case Qt::EditRole:
//        //            file()->setSide(static_cast<Side>(value.toBool()));
//        //            return true;
//        //        default:
//        //            return false;
//        //        }
//    default:
//        return false;
//    }
//}

//Qt::ItemFlags NodeLayer::flags(const QModelIndex& index) const
//{
//    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
//    switch (index.column()) {
//    case Name_:
//        return itemFlag /*| Qt::ItemIsUserCheckable*/;
//    case Layer_:
//        return itemFlag /*| Qt::ItemIsEditable*/;
//    default:
//        return itemFlag;
//    }
//}

//QVariant NodeLayer::data(const QModelIndex& index, int role) const
//{
//    if (file())
//        switch (index.column()) {
//        case Name_:
//            switch (role) {
//            case Qt::DisplayRole:
//                return name;
//            case Qt::ToolTipRole:
//                return name;
//                //                return file()->shortName() + "\n"
//                //                    + file()->name();
//                //            case Qt::CheckStateRole:
//                //                return layer->gig->isVisible() ? Qt::Checked : Qt::Unchecked;
//                //            case Qt::DecorationRole:
//                //                return QIcon::fromTheme("drill-path");
//                //            case Qt::UserRole:
//                //                return m_id;
//                //            default:
//                return QVariant();
//            case Qt::DecorationRole: {
//                QColor color(file()->color());
//                color.setAlpha(255);
//                QPixmap pixmap(22, 22);
//                pixmap.fill(Qt::transparent);
//                QPainter p(&pixmap);
//                p.setBrush(Dxf::dxfColors[layer->colorNumber]);
//                p.drawRect(3, 3, 15, 15);
//                return pixmap;
//            }
//            }
//        case Layer_:
//            //            switch (role) {
//            //            case Qt::DisplayRole:
//            //            case Qt::ToolTipRole:
//            //                return sideStrList[file()->side()];
//            //            case Qt::EditRole:
//            //                return static_cast<bool>(file()->side());
//            //            case Qt::UserRole:
//            //                return m_id;
//            //            default:
//            //                return QVariant();
//            //            }
//        default:
//            return QVariant();
//        }
//    return QVariant();
//}

//void NodeLayer::menu(QMenu* menu, TreeView* tv) const
//{
//    //    menu->addAction(QIcon::fromTheme("hint"), QObject::tr("&Hide other"), tv, &TreeView::hideOther);
//    //    if (!m_exFormatDialog) {
//    //        menu->addAction(QIcon::fromTheme("configure-shortcuts"), QObject::tr("&Edit Format"), [this] {
//    //            if (App::drillForm())
//    //                App::drillForm()->on_pbClose_clicked();
//    //            m_exFormatDialog = new ExcellonDialog(App::project()->file<Excellon::File>(m_id));
//    //            QObject::connect(m_exFormatDialog, &ExcellonDialog::destroyed, [&] { m_exFormatDialog = nullptr; });
//    //            m_exFormatDialog->show();
//    //        });
//    //    }
//    //    menu->addAction(QIcon::fromTheme("document-close"), QObject::tr("&Close"), tv, &TreeView::closeFile);
//}
}
