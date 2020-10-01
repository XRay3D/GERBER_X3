// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shhandler.h"
#include "doublespinbox.h"
#include "graphicsview.h"
#include <QFont>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtWidgets>

namespace Shapes {

void drawPos(QPainter* painter, const QPointF& pt1)
{
    QFont font;
    font.setPixelSize(16);
    const QString text = QString(::GlobalSettings::inch() ? "  X = %1 in\n"
                                                            "  Y = %2 in\n"
                                                          : "  X = %1 mm\n"
                                                            "  Y = %2 mm\n")
                             .arg(pt1.x() / (::GlobalSettings::inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg(pt1.y() / (::GlobalSettings::inch() ? 25.4 : 1.0), 4, 'f', 3, '0');

    const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, text);
    const double k = App::graphicsView()->scaleFactor();
    //    painter->drawLine(line);
    //    line.setLength(20.0 * k);
    //    line.setAngle(angle + 10);
    //    painter->drawLine(line);
    //    line.setAngle(angle - 10);
    //    painter->drawLine(line);
    // draw text
    //painter->setFont(font);
    //painter->drawText(textRect, Qt::AlignLeft, text);
    //    QPointF pt(pt1);
    //    if ((pt.x() + textRect.width() * k) > App::graphicsView()->visibleRegion().boundingRect().right())
    //        pt.rx() -= textRect.width() * k;
    //    if ((pt.y() - textRect.height() * k) < App::graphicsView()->visibleRegion().boundingRect().top())
    //        pt.ry() += textRect.height() * k;
    //    painter->translate(pt);
    painter->save();
    painter->scale(k, -k);
    int i = 0;
    for (QString txt : text.split('\n')) {
        QPainterPath path;
        path.addText(textRect.topLeft() + QPointF(textRect.left(), textRect.height() * 0.25 * ++i), font, txt);
        painter->setPen(QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawPath(path);
    }
    painter->restore();
}

Handler::Handler(Shape* shape, HType type)
    : shape(shape)
    , m_hType(type)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);
    switch (m_hType) {
    case Adder:
        setZValue(std::numeric_limits<double>::max());
        break;
    case Center:
        setZValue(std::numeric_limits<double>::max());
        break;
    case Corner:
        setZValue(std::numeric_limits<double>::max());
        break;
    }
    hhh.append(this);
}

Handler::~Handler() { hhh.removeOne(this); }

QRectF Handler::boundingRect() const { return rect(); }

void Handler::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    painter->setPen(Qt::NoPen);
    QColor c;
    switch (m_hType) {
    case Adder:
        c = QColor(Qt::yellow);
        break;
    case Center:
        c = QColor(Qt::red);
        break;
    case Corner:
        c = QColor(Qt::green);
        break;
    }
    if (option->state & QStyle::State_MouseOver) {
        drawPos(painter, pos());
    } else
        c.setAlpha(100);
    painter->setBrush(c);
    painter->drawEllipse(rect());
}

void Handler::setPos(const QPointF& pos)
{
    QGraphicsItem::setPos(pos);
    for (Handler* sh : hhh) { // прилипание
        if (sh->m_hType == Corner
            && sh->shape->isVisible()
            && QLineF(sh->pos(), pos).length() < App::graphicsView()->scaleFactor() * 20) {
            QGraphicsItem::setPos(sh->pos());
            return;
        }
    }
    if (m_hType == Center)
        return;
    QGraphicsItem::setPos(shape->calcPos(this));
}

Handler::HType Handler::hType() const { return m_hType; }

void Handler::setHType(const HType& value) { m_hType = value; }

QRectF Handler::rect() const
{
    const double scale = App::graphicsView()->scaleFactor();
    const double k = (m_hType == Center ? 5 : 5) * scale;
    const double s = k * 2;
    return { QPointF(-k, -k), QSizeF(s, s) };
}

void Handler::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    shape->m_scale = std::numeric_limits<double>::max();
    QGraphicsItem::mouseMoveEvent(event);
    setPos(GlobalSettings::getSnappedPos(pos(), event->modifiers()));
    if (m_hType == Center) {
        for (int i = 1, end = shape->handlers.size(); i < end; ++i)
            shape->handlers[i]->QGraphicsItem::setPos(pt[i] + pos() - pt.first());
        shape->redraw();
    } else {
        for (Handler* h : hhh) { // прилипание
            if (h != this
                && h->m_hType == Corner
                && h->shape->isVisible()
                && QLineF(h->pos(), pos()).length() < App::graphicsView()->scaleFactor() * 20) { // прилипание
                QGraphicsItem::setPos(h->pos());
            }
        }
        QGraphicsItem::setPos(shape->calcPos(this));
        shape->redraw();
    }
}

void Handler::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    shape->m_scale = std::numeric_limits<double>::max();
    QGraphicsItem::mousePressEvent(event);
    if (m_hType == Center) {
        pt.clear();
        for (Handler* item : shape->handlers)
            pt.append(item->pos());
    }
}

void Handler::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    //    QMenu menu;
    //    menu.addAction("EditPos", [event, this] {
    class Dialog : public QDialog {
    public:
        Dialog(Handler* h)
        {
            setWindowFlags(Qt::Popup);
            setModal(false);
            // clang-format off
            enum PType{ X, Y };
            // clang-format on
            auto dsbx = [h, this](PType type) {
                auto ds = new DoubleSpinBox(this);
                ds->setDecimals(3);
                ds->setRange(-1000, +1000);
                ds->setSuffix(QObject::tr(" mm"));
                ds->setValue(type == X ? h->pos().x() : h->pos().y());
                ds->connect(ds, qOverload<double>(&QDoubleSpinBox::valueChanged),
                    [h, type](auto val) {
                        if (type == X)
                            h->setPos({ val, h->pos().y() });
                        else
                            h->setPos({ h->pos().x(), val });
                        h->shape->calcPos(h);
                        h->shape->redraw();
                    });
                return ds;
            };

            auto gl = new QGridLayout(this);
            gl->addWidget(new QLabel("X:", this), 0, 0);
            gl->addWidget(new QLabel("Y:", this), 1, 0);
            gl->addWidget(dsbx(X), 0, 1);
            gl->addWidget(dsbx(Y), 1, 1);
            gl->setMargin(6);
        }
        virtual ~Dialog() { }
        void leaveEvent(QEvent*) override { reject(); };
    } d(this);
    d.move(event->screenPos());
    d.exec();
    //    });
    //    menu.exec(event->screenPos());
}
}
