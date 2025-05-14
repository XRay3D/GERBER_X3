#pragma once

#include <QStringBuilder>
#include <QVariant>
#include <QVector>
#include <array>

class TreeItem {
public:
    explicit TreeItem(
        const QString& Name,
        const QString& Type,
        const QVariant& Value,
        const QString& Attr,
        int Line)
        : itemData{QString{Name % "(" % Type % ")"}, Value, Attr, Line} { }

    enum {
        NameType,
        Value,
        IsAttr,
        FLine,
    };

    TreeItem() { };
    ~TreeItem();

    QVariant data(uint column, int role) const;
    TreeItem* child(uint number);
    TreeItem* parent();
    // bool insertChildren(int position, int count, int columns);
    // bool insertColumns(int position, int columns);
    // bool removeChildren(int position, int count);
    // bool removeColumns(int position, int columns);
    bool setData(uint column, const QVariant& value);
    int childCount() const;
    int childNumber() const;
    int columnCount() const;
    std::array<QVariant, 4> itemData;
    TreeItem* addItem(TreeItem* item) {
        return childItems.emplace_back(item)->parentItem = this, childItems.back();
    }

    std::vector<TreeItem*> childItems;

private:
    TreeItem* parentItem{};
};
