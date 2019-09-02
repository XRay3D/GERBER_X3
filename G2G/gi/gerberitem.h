#ifndef WORKITEM_H
#define WORKITEM_H

#include "graphicsitem.h"

namespace Gerber {
class File;
}

class GerberItem : public GraphicsItem {
public:
    explicit GerberItem(Paths& m_paths, Gerber::File* file);
    ~GerberItem() override /* = default*/;
    //    {
    //    if (dynamic_cast<const G::File*>(m_file)) {
    //        int index = m_file->groupedPaths().indexOf(m_paths);
    //        //qDebug() << "~GerberItem() index" << index;
    //        if (index > -1)
    //            m_file->groupedPaths().remove(index);
    //    }
    //    }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // GraphicsItem interface
    void redraw() override;
    Paths paths() const override;
    Paths& rPaths() override;
    //const Gerber::File* file() const;

private:
    Paths& m_paths;
};
#endif // WORKITEM_H
