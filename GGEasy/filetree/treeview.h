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
#pragma once
#include "filemodel.h"
#include <QTreeView>

class FileTreeView : public QTreeView {
    Q_OBJECT
    friend class MainWindow;

public:
    explicit FileTreeView(QWidget* parent = nullptr);
    ~FileTreeView() override ;
    void hideOther();
    void closeFile();
    void saveGcodeFile();

private:
    void updateTree();
    void updateIcons();
    FileModel* const m_model;

    void on_doubleClicked(const QModelIndex& index);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    QModelIndex m_menuIndex;
    void showExcellonDialog();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
};
