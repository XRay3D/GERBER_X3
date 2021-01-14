/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
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
    ~FileTreeView() override;
    void hideOther();
    void closeFile();
    void closeFiles();
    void setModel(QAbstractItemModel* model) override;

signals:
    void saveGCodeFile(int id);
    void saveGCodeFiles();
    void saveSelectedGCodeFiles();

private:
    void updateTree();
    void updateIcons();
    FileModel* m_model;

    void on_doubleClicked(const QModelIndex& index);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    int m_childCount = 0;
    QModelIndex m_menuIndex;
    void showExcellonDialog();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
};
