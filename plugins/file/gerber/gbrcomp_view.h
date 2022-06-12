/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include <QTreeView>

namespace Gerber {

class ComponentItem;

class ComponentsView : public QTreeView {
    Q_OBJECT
    ComponentItem* item = nullptr;

public:
    explicit ComponentsView(QWidget* parent = nullptr);
    ~ComponentsView();
    void setFile(int fileId);

private:
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
};

} // namespace Gerber
