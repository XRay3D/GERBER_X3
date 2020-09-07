#include "compview.h"
#include "compmodel.h"

ComponentsView::ComponentsView(QWidget* parent)
    : QTreeView(parent)
{
}

void ComponentsView::setFile(int fileId)
{
    setModel(new Gerber::ComponentsModel(fileId, this));
    expandAll();
}
