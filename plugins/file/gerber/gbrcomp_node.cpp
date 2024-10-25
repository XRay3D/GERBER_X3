// This is a personal academic project. Dear PVS-Studio, please check it.
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
#include "gbrcomp_node.h"
#include "gbrcomp_dialog.h"
#include "gbrcomp_item.h"
#include "gbrcomp_onent.h"

#include "abstract_file.h"
#include "gbr_types.h"

#include <QGraphicsScene>
#include <QSharedPointer>

namespace Gerber::Comp {

const Component dummy;

sNode::sNode(const QString& name)
    : component{dummy}
    , name(name)
    , item(nullptr) {
}

sNode::sNode(const Component& component)
    : component{component}
    , name("")
    , item(Dialog::scene()->addRect(
          component.componentitem()->boundingRect(),
          Qt::NoPen,
          component.componentitem()->file()->color()))

{
}

sNode::~sNode() {
    childItems.clear();
}

int sNode::row() const {
    if(parentItem_)
        for(int i = 0, size = parentItem_->childItems.size(); i < size; ++i)
            if(parentItem_->childItems[i].data() == this)
                return i;
    return 0;
}

sNode* sNode::child(int row) { return childItems.value(row).data(); }

sNode* sNode::parentItem() { return parentItem_; }

void sNode::setChild(int row, sNode* item) {
    if(item)
        item->parentItem_ = this;
    if(row < childItems.size())
        childItems[row].reset(item);
}

void sNode::append(sNode* item) {
    item->parentItem_ = this;
    childItems.append(QSharedPointer<sNode>(item));
}

void sNode::remove(int row) { childItems.removeAt(row); }

bool sNode::setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/) {
    //    switch (index.column()) {
    //        //    case Name_:
    //        //        switch (role) {
    //        //        case Qt::CheckStateRole:
    //        //            file()->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
    //        //            return true;
    //        //        default:
    //        //            return false;
    //        //        }
    //        //    case Layer_:
    //        //        switch (role) {
    //        //        case Qt::EditRole:
    //        //            file()->setSide(static_cast<Side>(value.toBool()));
    //        //            return true;
    //        //        default:
    //        //            return false;
    //        //        }
    //        //    case Other_:
    //        //        switch (role) {
    //        //        case Qt::CheckStateRole:
    //        //            current_ = value.value<Qt::CheckState>();
    //        //            return true;
    //        //        default:
    //        //            return false;
    //        //        }
    //    default:
    //        return false;
    //    }
    return false;
}

Qt::ItemFlags sNode::flags(const QModelIndex& /*index*/) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled /*| Qt::ItemNeverHasChildren*/ | Qt::ItemIsSelectable;
    return itemFlag;
}

QVariant sNode::data(const QModelIndex& index, int role) const {
    if(!name.isEmpty()) {
        if(role == Qt::DisplayRole && index.column() == 0)
            return name;
        return {};
    }

    static const QStringList mountType(GbrObj::tr("TH|SMD|BGA|Other").split('|'));
    if(role == Qt::DisplayRole) {
        switch(index.column()) {
        case 0: /* <field> Manufacturer. */
            return component.refdes();
        case 1: /* <field> Manufacturer part number. */
            return component.manufacturer().partNumber;
        case 2: /* <field> E.g. 220nF. */
            return component.value();
        case 3: /* (TH|SMD|BGA|Other) Mount type. */
            return mountType.value(component.mount());
        case 4: /* <field> Footprint name. It is strongly recommended to comply with the IPC-7351 footprint names and pin numbering for all standard components. */
            return component.footprintName();
        case 5: /* <field> Package name. It is strongly recommended to comply with the JEDEC JEP95 standard. */
            return component.package().name;
        case 6: /* <field> Package description. */
            return component.package().description;
        case 7: /* <decimal> Height, in the unit of the file. */
            return component.height();
        default:
            return {};
        }
    }
    return {};
}

int sNode::childCount() const { return childItems.count(); }

} // namespace Gerber::Comp
