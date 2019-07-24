#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include "filemodel.h"

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
class ExcellonDialog;

class TreeView : public QTreeView {
    Q_OBJECT
public:
    explicit TreeView(QWidget* parent = nullptr);
    ~TreeView();

private:
    void updateTree();
    void updateIcons();
    FileModel* m_model;

    void on_doubleClicked(const QModelIndex& index);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void hideOther();
    ExcellonDialog* m_exFormatDialog = nullptr;
    QModelIndex m_menuIndex;
    void closeFile();
    void saveGcodeFile();
    void showExcellonDialog();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
};

#endif // MYTREEVIEW_H
