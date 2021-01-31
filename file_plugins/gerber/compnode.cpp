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
#include "compnode.h"
#include "gbrcomponent.h"
#include "gbrtypes.h"

#include <QSharedPointer>

namespace Gerber {

const Component dummy;

ComponentsNode::ComponentsNode(const QString& name)
    : component(dummy)
    , name(name)
{
}

ComponentsNode::ComponentsNode(const Gerber::Component& component)
    : component(component)
    , name("")
{
}

ComponentsNode::~ComponentsNode()
{

    childItems.clear();
}

int ComponentsNode::row() const
{
    if (m_parentItem)
        for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
            if (m_parentItem->childItems[i].data() == this)
                return i;
    return 0;
}

ComponentsNode* ComponentsNode::child(int row) { return childItems.value(row).data(); }

ComponentsNode* ComponentsNode::parentItem() { return m_parentItem; }

void ComponentsNode::setChild(int row, ComponentsNode* item)
{
    if (item)
        item->m_parentItem = this;
    if (row < childItems.size()) {
        childItems[row].reset(item);
    }
}

void ComponentsNode::append(ComponentsNode* item)
{
    item->m_parentItem = this;
    childItems.append(QSharedPointer<ComponentsNode>(item));
}

void ComponentsNode::remove(int row) { childItems.removeAt(row); }

bool ComponentsNode::setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/)
{
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
    //        //            m_current = value.value<Qt::CheckState>();
    //        //            return true;
    //        //        default:
    //        //            return false;
    //        //        }
    //    default:
    //        return false;
    //    }
    return false;
}

Qt::ItemFlags ComponentsNode::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled /*| Qt::ItemNeverHasChildren*/ | Qt::ItemIsSelectable;
    return itemFlag;
}

QVariant ComponentsNode::data(const QModelIndex& index, int role) const
{
    if (!name.isEmpty()) {
        if (role == Qt::DisplayRole && index.column() == 0)
            return name;
        return {};
    }

    static const QStringList mountType(GbrObj::tr("TH|SMD|BGA|Other").split('|'));
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: /* <field> Manufacturer. */
            return component.refdes;
        case 1: /* <field> Manufacturer part number. */
            return component.manufacturer.partNumber;
        case 2: /* <field> E.g. 220nF. */
            return component.value;
        case 3: /* (TH|SMD|BGA|Other) Mount type. */
            return mountType.value(component.mount);
        case 4: /* <field> Footprint name. It is strongly recommended to comply with the IPC-7351 footprint names and pin numbering for all standard components. */
            return component.footprintName;
        case 5: /* <field> Package name. It is strongly recommended to comply with the JEDEC JEP95 standard. */
            return component.package.name;
        case 6: /* <field> Package description. */
            return component.package.description;
        case 7: /* <decimal> Height, in the unit of the file. */
            return component.height;
        default:
            return {};
        }
    }
    return {};
}

int ComponentsNode::childCount() const { return childItems.count(); }
}
