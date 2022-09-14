#include "shapepluginin.h"
#include "app.h"
#include "project.h"

namespace Shapes {

void Plugin::addPoint(const QPointF& point) {
    if (item == nullptr) {
        App::project()->addShape(item = createShape(point));
    } else if (!item->addPt(point)) {
        // NOTE finalizeShape();
        item->setSelected(true);
        item = nullptr;
    }
}

void Plugin::updPoint(const QPointF& point) {
    qDebug(__FUNCTION__);
    if (item)
        item->setPt(point);
}

void Plugin::finalizeShape() {
    qDebug(__FUNCTION__);
    if (item) {
        item->setSelected(true);
        item = nullptr;
    }
    emit actionUncheck();
}

Shapes::Plugin::Plugin() { App app; }

void Plugin::createMainMenu(QMenu& menu, FileTree::View* tv) {
    menu.addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete All Shapes"), [tv] {
        if (QMessageBox::question(tv, "", QObject::tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            tv->closeFiles();
    });
}

QString Plugin::folderName() const { return QObject::tr("Shapes"); }

const QJsonObject& Plugin::info() const { return info_; }

void Plugin::setInfo(const QJsonObject& info) { info_ = info; }

} // namespace Shapes

#include "moc_shapepluginin.cpp"
