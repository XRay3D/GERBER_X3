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

#include "ft_model.h"

#include <QTreeView>

namespace FileTree {

class View : public QTreeView {
    Q_OBJECT
    friend class ::MainWindow;

public:
    explicit View(QWidget* parent = nullptr);
    ~View() override;
    void hideOther();
    void closeFile();
    void closeFiles();
    void setModel(QAbstractItemModel* model) override;

signals:
    void saveGCodeFile(int id);
    void saveGCodeFiles(); // NOTE unused
    void saveSelectedGCodeFiles();

private:
    void updateTree();
    void updateIcons();
    Model* m_model;

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

} // namespace FileTree
