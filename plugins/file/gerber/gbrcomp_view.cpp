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
 *******************************************************************************/
#include "gbrcomp_view.h"
#include "gbrcomp_item.h"
#include "gbrcomp_model.h"
#include "gbrcomp_node.h"

#include "app.h"
#include "gbrcomp_onent.h"
#include "graphicsview.h"
#include <QDebug>
#include <QGraphicsRectItem>

namespace Gerber::Comp {

sView::sView(QWidget* parent)
    : QTreeView(parent) {
}

sView::~sView() {
    if(item)
        item->setSelected(false);
}

void sView::setFile(int fileId) {
    setModel(new sModel(fileId, this));
    expandAll();

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
        qDebug() << selected.size() << deselected.size();
        static QColor color;
        if(!selected.indexes().empty()) {
            auto node = reinterpret_cast<sNode*>(selected.indexes().front().internalPointer());
            if(node->item) {
                color = node->item->brush().color();
                node->item->setBrush(Qt::white);
                item = node->component.componentitem();
                item->setSelected(true);
                App::graphicsView().fitInView(node->item->boundingRect());
            }
        }

        if(!deselected.indexes().empty()) {
            auto node = reinterpret_cast<sNode*>(deselected.indexes().front().internalPointer());
            if(node->item) {
                node->item->setBrush(color);
                node->component.componentitem()->setSelected(false);
            }
        }
    });
}

void sView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {

    QTreeView::selectionChanged(selected, deselected);
}

} // namespace Gerber::Comp

#include "moc_gbrcomp_view.cpp"
