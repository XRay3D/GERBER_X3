#ifndef THERMALMODEL_H
#define THERMALMODEL_H

#include "thermalpreviewitem.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <myclipper.h>

class ThermalNode {

public:
    explicit ThermalNode(const QIcon& icon, const QString& name, double angle, double tickness, int count, const IntPoint& pos, ThermalPreviewItem* item)
        : container(false)
        , icon(icon)
        , name(name)
        , m_angle(angle)
        , m_tickness(tickness)
        , m_count(count)
        , m_pos(pos)
        , m_item(item)
    {
        item->m_node = this;
    }
    explicit ThermalNode(const QIcon& icon, const QString& name)
        : container(true)
        , icon(icon)
        , name(name)
        , m_angle(0)
        , m_tickness(0.5)
        , m_count(4)
    {
    }

    ~ThermalNode()
    {
        childItems.clear();
    }

    ThermalNode* child(int row) const
    {
        return childItems.value(row).data();
    }

    ThermalNode* parentItem()
    {
        return m_parentItem;
    }

    int childCount() const
    {
        return childItems.count();
    }

    int row() const
    {
        if (m_parentItem)
            for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
                if (m_parentItem->childItems[i].data() == this)
                    return i;
        return 0;
    }

    void append(ThermalNode* item)
    {
        item->m_parentItem = this;
        childItems.append(QSharedPointer<ThermalNode>(item));
    }
    void remove(int row) { childItems.removeAt(row); }

    bool setData(const QModelIndex& index, const QVariant& value, int role)
    {
        switch (role) {
        case Qt::EditRole:
            if (container) {
                switch (index.column()) {
                case 0:
                case 1:
                    return false;
                case 2:
                    for (QSharedPointer<ThermalNode> item : childItems) {
                        item->setData(index, value, role);
                    }
                    return m_angle = value.toDouble();
                    return true;
                case 3:
                    for (QSharedPointer<ThermalNode> item : childItems) {
                        item->setData(index, value, role);
                    }
                    return m_tickness = value.toDouble();
                    return true;
                case 4:
                    for (QSharedPointer<ThermalNode> item : childItems) {
                        item->setData(index, value, role);
                    }
                    return m_count = value.toInt();
                    return true;
                }
                return false;
            } else {
                switch (index.column()) {
                case 0:
                case 1:
                    return false;
                case 2:
                    m_item->setAngle(value.toDouble());
                    return true;
                case 3:
                    m_item->setTickness(value.toDouble());
                    return true;
                case 4:
                    m_item->setCount(value.toInt());
                    return true;
                }
            }
            return false;
        case Qt::CheckStateRole:
            if (!index.column()) {
                m_checkState = value.toBool() ? Qt::Checked : Qt::Unchecked;
                if (m_item) {
                    if (value.toBool()) {
                        m_item->setFlag(QGraphicsItem::ItemIsSelectable, value.toBool());
                        m_item->setSelected(selected);
                    } else {
                        selected = m_item->isSelected();
                        m_item->setFlag(QGraphicsItem::ItemIsSelectable, value.toBool());
                    }
                }
                for (QSharedPointer<ThermalNode> item : childItems) {
                    item->setData(index, value, role);
                }
                return true;
            }
            return false;
        default:
            break;
        }
        return false;
    }

    Qt::ItemFlags flags(const QModelIndex& index) const
    {
        Qt::ItemFlags flags = Qt::ItemIsEnabled;
        if (!index.column())
            flags |= Qt::ItemIsUserCheckable;
        if (index.column() > 1 /*&& !container*/)
            flags |= Qt::ItemIsEditable;
        if (!container)
            flags |= Qt::ItemNeverHasChildren;
        if (m_checkState == Qt::Checked)
            flags |= Qt::ItemIsSelectable;
        return flags;
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            if (container) {
                switch (index.column()) {
                case 0:
                    return name;
                case 1:
                    return QString("%1 : %2").arg(m_pos.X * dScale).arg(m_pos.Y * dScale).replace('.', ',');
                case 2:
                    return m_angle;
                case 3:
                    return m_tickness;
                case 4:
                    return m_count;
                }
            } else {
                switch (index.column()) {
                case 0:
                    return name;
                case 1:
                    return QString("%1 : %2").arg(m_pos.X * dScale).arg(m_pos.Y * dScale).replace('.', ',');
                case 2:
                    return m_item->angle();
                case 3:
                    return m_item->tickness();
                case 4:
                    return m_item->count();
                }
            }
            return QVariant();
        case Qt::DecorationRole:
            if (!index.column())
                return icon;
            return QVariant();
        case Qt::CheckStateRole:
            if (!index.column())
                return m_checkState;
            return QVariant();
        case Qt::TextAlignmentRole:
            if (index.column())
                return Qt::AlignCenter;
            return QVariant();
        default:
            break;
        }
        return QVariant();
    }

    double angle() const { return m_angle; }
    double tickness() const { return m_tickness; }
    int count() const { return m_count; }
    IntPoint pos() const { return m_pos; }
    ThermalPreviewItem* item() const { return m_item; }
    bool createFile() const { return m_checkState == Qt::Checked && !container; }
    void disable()
    {
        m_checkState = Qt::Unchecked;
        selected = false;
        m_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
    }
    void enable()
    {
        m_checkState = Qt::Checked;
        selected = true;
        m_item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        m_item->setSelected(true);
    }

    ThermalNode(const ThermalNode&) = delete;
    ThermalNode& operator=(const ThermalNode&) = delete;

private:
    bool container = false;
    const QIcon icon;
    const QString name;
    double m_angle;
    double m_tickness;
    int m_count;
    const IntPoint m_pos;

    ThermalPreviewItem* m_item = nullptr;
    bool selected = false;

    ThermalNode* m_parentItem = nullptr;
    QList<QSharedPointer<ThermalNode>> childItems;
    Qt::CheckState m_checkState = Qt::Checked;
};

class ThermalModel : public QAbstractItemModel {
    Q_OBJECT
    friend class ThermalForm;

    //    typedef struct Row {
    //        Row(const QString& name, const QIcon& icon)
    //            : name{ name }
    //            , icon{ icon }
    //        {
    //        }
    //        QString name;
    //        QIcon icon;
    //        bool create = false;
    //    } Row;
    //    QList<Row> m_data;

    QIcon repaint(QColor color, const QIcon& icon) const;

    ThermalNode* rootItem = nullptr;
    QList<ThermalNode*> m_data;

public:
    explicit ThermalModel(QObject* parent = nullptr);
    ~ThermalModel() override;

    ThermalNode* appendRow(const QIcon& icon, const QString& name);
    //QModelIndex createIndex2(int arow, int acolumn, quintptr aid) const { return createIndex(arow, acolumn, aid); }

    //signals:
    //public slots:

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;

    //    virtual QVariant data(const QModelIndex& index, int role) const override;
    //    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    //    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    //    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;

private:
    ThermalNode* getItem(const QModelIndex& index) const;
};

#endif // THERMALMODEL_H
