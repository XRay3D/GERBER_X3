#pragma once

#include "filemodel.h"

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>

class TreeView : public QTreeView {
    Q_OBJECT
    friend class MainWindow;

public:
    explicit TreeView(QWidget* parent = nullptr);
    ~TreeView() override = default;
    void hideOther();
    void closeFile();
    void saveGcodeFile();

private:
    void updateTree();
    void updateIcons();
    FileModel* m_model;

    void on_doubleClicked(const QModelIndex& index);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    //void hideOther();

    QModelIndex m_menuIndex;
    //void closeFile();
    //void saveGcodeFile();
    void showExcellonDialog();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
};
