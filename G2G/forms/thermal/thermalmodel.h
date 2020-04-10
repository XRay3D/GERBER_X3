#pragma once
#ifndef THERMALMODEL_H
#define THERMALMODEL_H

#include "thermalnode.h"

#include <QAbstractItemModel>

class ThermalModel : public QAbstractItemModel {
    Q_OBJECT
    friend class ThermalForm;
    friend class ThermalNode;

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
    enum {
        ThName,
        ThPos,
        ThGapAngle,
        ThGapThickness,
        ThGapCount,
    };

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
