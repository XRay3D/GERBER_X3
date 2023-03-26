#include "shapepluginin.h"
#include "app.h"
#include "project.h"

namespace Shapes {
void Plugin::addPoint(const QPointF& point) {
    if (item == nullptr)
        item = createShape(point), App::project()->addShape(item);
    else if (!item->addPt(point))
        item->setSelected(true), item = nullptr;
}

void Plugin::updPoint(const QPointF& point) {
    if (item)
        item->setPt(point);
}

void Plugin::finalizeShape() {
    if (item)
        item->setSelected(true), item = nullptr;
    emit actionUncheck();
}

Shapes::Plugin::Plugin() { App app; }

void Plugin::createMainMenu(QMenu& menu, FileTree_::View* tv) {
    menu.addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete All Shapes"), [tv] {
        if (QMessageBox::question(tv, "", QObject::tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            tv->closeFiles();
    });
}

QString Plugin::folderName() const { return QObject::tr("Shapes"); }

} // namespace Shapes

#include "moc_shapepluginin.cpp"
