/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "compview.h"
#include "compmodel.h"

#include "leakdetector.h"

ComponentsView::ComponentsView(QWidget* parent)
    : QTreeView(parent)
{
}

void ComponentsView::setFile(int fileId)
{
    setModel(new Gerber::ComponentsModel(fileId, this));
    expandAll();
}
