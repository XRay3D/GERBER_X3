#pragma once
//#ifndef DRILLMODEL_H
//#define DRILLMODEL_H

#include <QAbstractTableModel>
#include <QIcon>

enum {
    tAperture,
    tTool,
};

class DrillModel : public QAbstractTableModel {
    Q_OBJECT

    using Row = struct Row {
        Row(const QString& name, const QIcon& icon, int id)
            : name { name, "" }
            , icon { icon, QIcon() }
            , apToolId(id)
            , toolId(-1)
        {
        }
        QString name[2];
        QIcon icon[2];
        int apToolId;
        int toolId;
        bool isSlot = false;
        bool create = false;
    };
    QList<Row> m_data;
    int m_type;

signals:
    void set(int, bool);

public:
    DrillModel(int type, QObject* parent = nullptr);
    void appendRow(const QString& name, const QIcon& icon, int id);
    void setToolId(int row, int id);
    int toolId(int row);
    void setSlot(int row, bool slot);
    bool isSlot(int row);
    int apertureId(int row);
    bool create(int row) const;
    void setCreate(int row, bool create);
    void setCreate(bool create);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};
//#endif // DRILLMODEL_H
