/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#include "mvector.h"
#include "tool_model.h"
#include <QTreeView>

class QPushButton;

class ToolTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit ToolTreeView(QWidget* parent = nullptr);
    ~ToolTreeView() override = default;
    void updateItem();
    void setButtons(const mvector<QPushButton*>& buttons);

signals:
    void itemSelected(ToolItem* item);

private:
    void newGroup();
    void newTool();
    void deleteItem();
    void copyTool();

    void updateActions();
    ToolModel* model_;
    enum {
        Copy,
        Delete,
        New,
        NewGroup,
    };
    mvector<QPushButton*> buttons_;
};
