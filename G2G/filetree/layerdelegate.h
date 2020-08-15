#pragma once
//#ifndef LAYERDELEGATE_H
//#define LAYERDELEGATE_H

#include <QStyledItemDelegate>

class LayerDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    LayerDelegate(QObject* parent = nullptr);
    ~LayerDelegate() override = default;

public:
    // QAbstractItemDelegate interface
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    void emitCommitData();
};

class RadioDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    RadioDelegate(QObject* parent = nullptr);
    ~RadioDelegate() override = default;

    // QAbstractItemDelegate interface
public:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private slots:
    void commitAndCloseEditor();
};
//#endif // LAYERDELEGATE_H
