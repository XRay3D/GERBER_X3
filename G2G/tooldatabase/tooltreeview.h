#pragma once

#include "toolmodel.h"
#include <QTreeView>
#include <QVector>

class QPushButton;

class ToolTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit ToolTreeView(QWidget* parent = nullptr);
    ~ToolTreeView() override = default;
    void updateItem();
    void setButtons(const QVector<QPushButton*>& buttons);

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
    QVector<QPushButton*> m_buttons;
};
