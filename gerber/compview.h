#pragma once

#include <QTreeView>

class ComponentsView : public QTreeView {
    Q_OBJECT
public:
    explicit ComponentsView(QWidget* parent = nullptr);
    void setFile(int fileId);
signals:
};
