// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shtext.h"
#include "shandler.h"
#include "shtextdialog.h"
#include <mainwindow.h>
#include <scene.h>

namespace Shapes {
Text::Text(QPointF pt1)
    : text("text")
{
    m_paths.resize(1);
    sh = { new Handler(this, true) };
    sh.first()->setPos(pt1);

    redraw();
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());

    App::scene()->addItem(this);
    App::scene()->addItem(sh.first());
}

Text::~Text() { }

void Text::redraw()
{
    QPainterPath p;
    QFont f;

    f.fromString(font);
    f.setPointSizeF(100);
    QFontMetrics fm(f);
    p.addText(QPointF(), f, text);

    if (Area(m_paths.first()) < 0)
        ReversePath(m_paths.first());

    m_shape = QPainterPath();
    QMatrix matrix;

    auto r = p.boundingRect();

    const auto k2 = fm.capHeight();
    const auto k = height / fm.ascent(); // k2/*r.height()*/;
    //r.setBottomRight(r.bottomRight() * k);

    QPointF tr;
    switch (centerAlign) {
    case 1:
        //ui->rb_bc->setChecked(true);
        tr.rx() -= r.width() * 0.5;
        tr.ry();
        break;
    case 2:
        //ui->rb_bl->setChecked(true);
        tr.rx();
        tr.ry();
        break;
    case 3:
        //ui->rb_br->setChecked(true);
        tr.rx() -= r.width();
        tr.ry();
        break;
    case 4:
        //ui->rb_c->setChecked(true);
        tr.rx() -= r.width() * 0.5;
        tr.ry() -= k2 /*r.height()*/ * 0.5;
        break;
    case 5:
        //ui->rb_lc->setChecked(true);
        tr.rx();
        tr.ry() -= k2 /*r.height()*/ * 0.5;
        break;
    case 6:
        //ui->rb_rc->setChecked(true);
        tr.rx() -= r.width();
        tr.ry() -= k2 /*r.height()*/ * 0.5;
        break;
    case 7:
        //ui->rb_tc->setChecked(true);
        tr.rx() -= r.width() * 0.5;
        tr.ry() -= k2 /*r.height()*/;
        break;
    case 8:
        //ui->rb_tl->setChecked(true);
        tr.rx();
        tr.ry() -= k2 /*r.height()*/;
        break;
    case 9:
        //ui->rb_tr->setChecked(true);
        tr.rx() -= r.width();
        tr.ry() -= k2 /*r.height()*/;
        break;
    }

    matrix.translate(-r.left() * k, 0 /*r.bottom() * k*/);
    matrix.translate(tr.x() * k, tr.y() * k);
    matrix.scale(k, -k);
    {
        QPainterPath p2;
        for (auto& sp : p.toSubpathPolygons()) {
            p2.addPolygon(sp);
        }
        p = p2;
        p2 = QPainterPath();
        for (auto& sp : p.toSubpathPolygons(matrix)) {
            p2.addPolygon(sp);
        }
        p = p2;
    }
    matrix.reset();
    matrix.translate(sh.first()->pos().x(), sh.first()->pos().y());
    matrix.rotate(angle - 360);

    m_paths.clear();

    for (auto& sp : p.toSubpathPolygons(matrix)) {
        m_shape.addPolygon(sp);
        m_paths.append(toPath(sp));
    }
    m_scale = std::numeric_limits<double>::max();

    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QPointF Text::calcPos(Handler* sh) const { return sh->pos(); }

void Text::write(QDataStream& stream) const
{
    stream << text;
    stream << font;
    stream << angle;
    stream << height;
    stream << centerAlign;
}

void Text::read(QDataStream& stream)
{
    stream >> text;
    stream >> font;
    stream >> angle;
    stream >> height;
    stream >> centerAlign;
}

void Shapes::Text::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    ShTextDialog dlg(this, App::mainWindow());
    if (!dlg.exec()) {
        text = dlg.text_;
        font = dlg.font_;
        angle = dlg.angle_;
        height = dlg.height_;
        centerAlign = dlg.centerAlign_;
    }
    qDebug() << font;
    redraw();
}

}
