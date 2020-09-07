#pragma once

#include <QModelIndex>

namespace Gerber {

class Component;

class ComponentsNode {
    ComponentsNode(const ComponentsNode&) = delete;
    ComponentsNode& operator=(const ComponentsNode&) = delete;

    const Gerber::Component& component;
    const QString name;

public:
    ComponentsNode(const QString& name);
    ComponentsNode(const Gerber::Component& component);
    virtual ~ComponentsNode();

    ComponentsNode* child(int row);
    ComponentsNode* parentItem();

    void setChild(int row, ComponentsNode* item);

    int childCount() const;
    int row() const;

    void append(ComponentsNode* item);
    void remove(int row);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    //    virtual void menu(QMenu* menu, TreeView* tv) const = 0;

    //    enum {
    //        Name,
    //        Layer,
    //        Other
    //    };

protected:
    //    const int m_id;
    //    const int m_type;

    //    const QStringList tbStrList;
    ComponentsNode* m_parentItem = nullptr;
    QList<QSharedPointer<ComponentsNode>> childItems;
    //    inline AbstractFile* file() const { return App::project()->file(m_id); }
    //    inline Shapes::Shape* shape() const { return App::project()->aShape(m_id); }
    //Qt::CheckState m_checkState = Qt::Checked;
};

}
