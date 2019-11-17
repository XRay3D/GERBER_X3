#include "abstractnode.h"
#include "mainwindow.h"
#include "project.h"

AbstractNode::AbstractNode(int id)
    : m_id(id)
{
    //    if (MainWindow::closeAllAct()) {
    //        MainWindow::closeAllAct()->setEnabled(true);
    //        MainWindow::exportPdfAct()->setEnabled(true);
    //    }
}

AbstractNode::~AbstractNode()
{
    //    if (MainWindow::closeAllAct()) {
    //        MainWindow::closeAllAct()->setEnabled(Project::isEmpty());
    //        MainWindow::exportPdfAct()->setEnabled(Project::isEmpty());
    //    }
    if (m_id != -1) {
        QGraphicsScene* scene = Project::file(m_id)->itemGroup()->first()->scene();
        if (scene) {
            scene->setSceneRect(scene->itemsBoundingRect());
            scene->update();
        }
        Project::deleteFile(m_id);
    }
    childItems.clear();
}

int AbstractNode::row() const
{
    if (m_parentItem)
        for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
            if (m_parentItem->childItems[i].data() == this)
                return i;
    return 0;
}

AbstractNode* AbstractNode::child(int row) { return childItems.value(row).data(); }

AbstractNode* AbstractNode::parentItem() { return m_parentItem; }

void AbstractNode::setChild(int row, AbstractNode* item)
{
    if (item)
        item->m_parentItem = this;
    if (row < childItems.size()) {
        childItems[row].reset(item);
    }
}

void AbstractNode::append(AbstractNode* item)
{
    item->m_parentItem = this;
    childItems.append(QSharedPointer<AbstractNode>(item));
}

void AbstractNode::remove(int row) { childItems.removeAt(row); }

int AbstractNode::childCount() const { return childItems.count(); }
