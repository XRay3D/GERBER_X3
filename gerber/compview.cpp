#include "compview.h"
#include "compmodel.h"

#include "leakdetector.h"

ComponentsView::ComponentsView(QWidget* parent)
    : QTreeView(parent)
{
}

void ComponentsView::setFile(int fileId)
{
    setModel(new Gerber::ComponentsModel(fileId, this));
    expandAll();
}
