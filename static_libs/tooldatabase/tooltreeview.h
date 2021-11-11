/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "toolmodel.h"
#include <QTreeView>
#include "mvector.h"

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
    ToolModel* m_model;
    enum {
        Copy,
        Delete,
        New,
        NewGroup,
    };
    mvector<QPushButton*> m_buttons;
};
