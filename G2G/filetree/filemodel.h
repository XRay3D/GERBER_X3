#pragma once
#ifndef MODEL_H
#define MODEL_H

#include <QAbstractItemModel>
    
enum RootNodes {
    NodeGerberFiles,
    NodeDrillFiles,
    NodeToolPath,
    NodeSpecial,
    NodeCount,
};

class AbstractFile;
class AbstractNode;

class FileModel : public QAbstractItemModel {
    Q_OBJECT
    AbstractNode* rootItem;
    friend class Project;

signals:
    void updateActions();
    void select(const QModelIndex&);

public:
    explicit FileModel(QObject* parent = nullptr);
    ~FileModel() override;

    static void closeProject();

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

    static FileModel* instance();

    // Drag and Drop
    //    QStringList mimeTypes() const override;
    //    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    //    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    //    Qt::DropActions supportedDragActions() const override;
    //    Qt::DropActions supportedDropActions() const override;
    //    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

private:
    const QString mimeType;

    static void addFile(AbstractFile* file);
    static FileModel* m_instance;
    AbstractNode* getItem(const QModelIndex& index) const;
};

#endif // MODEL_H
