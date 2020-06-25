#pragma once
#ifndef TREEITEM_H
#define TREEITEM_H

#include <QAbstractItemModel>
#include <QSharedPointer>
#include <QVariant>
#include <app.h>
#include <project.h>

class AbstractFile;
class TreeView;

class AbstractNode {
public:
    explicit AbstractNode(int id);
    virtual ~AbstractNode();

    AbstractNode* child(int row);
    AbstractNode* parentItem();

    void setChild(int row, AbstractNode* item);

    int childCount() const;
    int row() const;

    void append(AbstractNode* item);
    void remove(int row);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) = 0;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const = 0;
    virtual QVariant data(const QModelIndex& index, int role) const = 0;
    virtual void menu(QMenu* menu, TreeView* tv) const = 0;

    enum {
        Name,
        Layer,
        Other
    };

    AbstractNode(const AbstractNode&) = delete;
    AbstractNode& operator=(const AbstractNode&) = delete;

protected:
    const int m_id;
    const QStringList tbStrList;
    AbstractNode* m_parentItem = nullptr;
    QList<QSharedPointer<AbstractNode>> childItems;
    inline AbstractFile* file() const { return App::project()->file(m_id); }
    //Qt::CheckState m_checkState = Qt::Checked;
};

#endif // TREEITEM_H
