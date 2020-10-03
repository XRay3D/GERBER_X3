#pragma once

#include <QAbstractItemModel>

class ThermalNode;

class ThermalModel : public QAbstractItemModel {
    Q_OBJECT
    friend class ThermalForm;
    friend class ThermalNode;

    QIcon repaint(QColor color, const QIcon& icon) const;

    ThermalNode* const rootItem = nullptr;
    QList<ThermalNode*> m_data;

public:
    enum {
        Name,
        Position,
        GapAngle,
        apThickness,
        GapCount,
    };

    explicit ThermalModel(QObject* parent = nullptr);
    ~ThermalModel() override;

    ThermalNode* appendRow(const QIcon& icon, const QString& name);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
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
