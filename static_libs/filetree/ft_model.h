/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include <QAbstractItemModel>

class FileInterface;

namespace Shapes {
class Shape;
}
class Project;

namespace FileTree {

class Node;

class Model : public QAbstractItemModel {
    Q_OBJECT
    Node* rootItem;
    struct Pair {
        Node* node;
        int type;
    };
    std::map<int, Pair> mapNode;
    friend class ::Project;
    friend class Node;
    int rotId = -1;
signals:
    void updateActions();
    void select(const QModelIndex&);

public:
    enum RootNodes {
        GerberFiles,
        DrillFiles,
        ToolPath,
        DxfPath,
        Shapes,
        NodeCount,
    };

    explicit Model(QObject* parent = nullptr);
    ~Model() override;

    void closeProject();

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;

    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;
    // костыли
    inline void beginInsertRows_(const QModelIndex& parent, int first, int last) { QAbstractItemModel::beginInsertRows(parent, first, last); }
    inline void endInsertRows_() { QAbstractItemModel::endInsertRows(); }
    inline void beginRemoveRows_(const QModelIndex& parent, int first, int last) { QAbstractItemModel::beginRemoveRows(parent, first, last); }
    inline void endRemoveRows_() { QAbstractItemModel::endRemoveRows(); }
    inline QModelIndex createIndex_(int row, int column, quintptr id) const { return QAbstractItemModel::createIndex(row, column, id); }

    // Drag and Drop
    //    QStringList mimeTypes() const override;
    //    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    //    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    //    Qt::DropActions supportedDragActions() const override;
    //    Qt::DropActions supportedDropActions() const override;
    //    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;
    Node* getItem(const QModelIndex& index) const;

private:
    const QString mimeType;

    void addFile(FileInterface* file);
    void addShape(Shapes::Shape* shape);
};
}
#include "app.h"
