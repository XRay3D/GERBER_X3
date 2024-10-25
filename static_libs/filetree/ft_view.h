/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
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

    void closeAllFiles(uint32_t type);

    void setModel(QAbstractItemModel* model) override;

signals:
    void saveGCodeFile(int32_t id);
    void saveGCodeFiles(); // NOTE unused
    void saveSelectedGCodeFiles();

private:
    void updateTree();
    void updateIcons();
    Model* model_;

    void on_doubleClicked(const QModelIndex& index);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    int childCount_ = 0;
    QModelIndex menuIndex_;
    void showExcellonDialog();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
};

} // namespace FileTree
