// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "compview.h"
#include "compitem.h"
#include "compmodel.h"
#include "compnode.h"

#include "app.h"
#include "gbrcomponent.h"
#include "graphicsview.h"
#include <QDebug>
#include <QGraphicsRectItem>

namespace Gerber {

ComponentsView::ComponentsView(QWidget* parent)
    : QTreeView(parent) {
}

ComponentsView::~ComponentsView() {
    if (item)
        item->setSelected(false);
}

void ComponentsView::setFile(int fileId) {
    setModel(new ComponentsModel(fileId, this));
    expandAll();

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
        qDebug() << selected.size() << deselected.size();
        static QColor color;
        if (!selected.indexes().empty()) {
            auto node = reinterpret_cast<ComponentsNode*>(selected.indexes().front().internalPointer());
            if (node->item) {
                color = node->item->brush().color();
                node->item->setBrush(Qt::white);
                item = node->component.componentitem();
                item->setSelected(true);
                App::graphicsView()->fitInView(node->item->boundingRect());
            }
        }

        if (!deselected.indexes().empty()) {
            auto node = reinterpret_cast<ComponentsNode*>(deselected.indexes().front().internalPointer());
            if (node->item) {
                node->item->setBrush(color);
                node->component.componentitem()->setSelected(false);
            }
        }
    });
}

void ComponentsView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    //qDebug() << selected.value(0) << deselected.value(0);
    QTreeView::selectionChanged(selected, deselected);
}

} // namespace Gerber
