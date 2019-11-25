#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include "toolmodel.h"

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>

class ToolTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit ToolTreeView(/*QVector<QPushButton*> buttons,*/ QWidget* parent = nullptr);
    ~ToolTreeView() override = default;
    void updateItem();

    void setButtons(const QVector<QPushButton*>& buttons);

signals:
    void itemSelected(ToolItem* item);

public slots:

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
    QVector<QPushButton*> m_buttons;
};

#endif // MYTREEVIEW_H
