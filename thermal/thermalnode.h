#pragma once

#include "thermalpreviewitem.h"
#include <QIcon>
#include <QModelIndex>

class ThermalModel;

class ThermalNode {
public:
    explicit ThermalNode(const QIcon& icon, const QString& name, double angle, double tickness, int count, const IntPoint& pos, ThermalPreviewItem* item);
    explicit ThermalNode(const QIcon& icon, const QString& name);
    explicit ThermalNode(ThermalModel* _model) { model = _model; }

    ~ThermalNode();

    ThermalNode* child(int row) const;

    ThermalNode* parentItem();

    int childCount() const;

    int row() const;

    void append(ThermalNode* item);
    void remove(int row);

    bool setData(const QModelIndex& index, const QVariant& value, int role);

    Qt::ItemFlags flags(const QModelIndex& index) const;

    QVariant data(const QModelIndex& index, int role) const;

    double angle() const;
    double tickness() const;
    int count() const;
    IntPoint pos() const;
    ThermalPreviewItem* item() const;
    bool createFile() const;
    void disable();
    void enable();

    ThermalNode(const ThermalNode&) = delete;
    ThermalNode& operator=(const ThermalNode&) = delete;

    bool isChecked() const;
    QModelIndex index() const;

private:
    const bool container = false;
    const QIcon icon;
    const QString name;
    double m_angle;
    double m_tickness;
    int m_count;
    const IntPoint m_pos;

    ThermalPreviewItem* const m_item = nullptr;

    ThermalNode* m_parentItem = nullptr;
    QList<QSharedPointer<ThermalNode>> childItems;
    bool m_checked = false;

    inline static ThermalModel* model;
    inline static const Qt::CheckState chState[] {
        Qt::Unchecked, // index 0
        Qt::Unchecked, // index 1
        Qt::Checked, // index 2
        Qt::PartiallyChecked // index 3
    };
};
