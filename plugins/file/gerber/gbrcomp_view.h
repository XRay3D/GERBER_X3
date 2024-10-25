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
#pragma once
#include <QTreeView>

namespace Gerber::Comp {

class Item;

class sView : public QTreeView {
    Q_OBJECT
    Item* item = nullptr;

public:
    explicit sView(QWidget* parent = nullptr);
    ~sView();
    void setFile(int fileId);

private:
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
};

} // namespace Gerber::Comp
