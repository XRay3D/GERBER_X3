#include "scene.h"
#include "graphicsview.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QPainter>
#include <QPdfWriter>
#include <QtMath>
#include <gi/graphicsitem.h>
#include <settings.h>

Scene* Scene::m_self = nullptr;

Scene::Scene(QObject* parent)
    : QGraphicsScene(parent)
    , m_drawPdf(false)
{
    m_self = this;
}

Scene::~Scene()
{
    m_self = nullptr;
}

void Scene::RenderPdf()
{
    QString curFile = QFileDialog::getSaveFileName(nullptr, tr("Save PDF file"), "File", tr("File(*.pdf)"));
    if (curFile.isEmpty())
        return;

    m_drawPdf = true;

    QRectF rect;

    for (QGraphicsItem* item : items())
        if (item->isVisible() && !item->boundingRect().isNull())
            rect |= item->boundingRect();

    //QRectF rect(QGraphicsScene::itemsBoundingRect());
    QSizeF size(rect.size());

    qDebug() << size << rect;

    QPdfWriter pdfWriter(curFile);
    pdfWriter.setPageSizeMM(size);
    pdfWriter.setMargins({ 0, 0, 0, 0 });
    pdfWriter.setResolution(1000000);

    QPainter painter(&pdfWriter);
    painter.setTransform(QTransform().scale(1.0, -1.0));
    painter.translate(0, -(pdfWriter.resolution() / 25.4) * size.height());
    render(&painter,
        QRectF(0, 0, pdfWriter.width(), pdfWriter.height()),
        rect, Qt::IgnoreAspectRatio);

    m_drawPdf = false;
}

QRectF Scene::itemsBoundingRect()
{
    m_drawPdf = true;
    QRectF rect(QGraphicsScene::itemsBoundingRect());
    m_drawPdf = false;
    return rect;
}

bool Scene::drawPdf()
{
    if (m_self)
        return m_self->m_drawPdf;
    return false;
}

QList<QGraphicsItem*> Scene::selectedItems()
{
    if (m_self)
        return m_self->QGraphicsScene::selectedItems();
    return QList<QGraphicsItem*>();
}

void Scene::addItem(QGraphicsItem* item)
{
    if (m_self)
        m_self->QGraphicsScene::addItem(item);
}

QList<QGraphicsItem*> Scene::items(Qt::SortOrder order)
{
    if (m_self)
        return m_self->QGraphicsScene::items(order);
    return QList<QGraphicsItem*>();
}

void Scene::update()
{
    if (m_self)
        m_self->QGraphicsScene::update();
}

void Scene::setCross(const QPointF& cross)
{
    m_cross = cross;
    update();
}

void Scene::setCross2(const QPointF& cross2)
{
    m_cross2 = cross2;
}

void Scene::drawRuller(QPainter* painter)
{
    const QPointF pt1(m_cross2);
    const QPointF pt2(m_cross);
    QLineF line(pt2, pt1);
    const QRectF rect(
        QPointF(qMin(pt1.x(), pt2.x()), qMin(pt1.y(), pt2.y())),
        QPointF(qMax(pt1.x(), pt2.x()), qMax(pt1.y(), pt2.y())));
    const double length = line.length();
    const double angle = line.angle();

    QFont font;
    font.setPixelSize(16);
    const QString text = QString("  ∆X = %1 mm\n"
                                 "  ∆Y = %2 mm\n"
                                 "  ∆ / = %3 mm\n"
                                 "  %4°")
                             .arg(rect.width(), 4, 'f', 3, '0')
                             .arg(rect.height(), 4, 'f', 3, '0')
                             .arg(length, 4, 'f', 3, '0')
                             .arg(360.0 - (angle > 180.0 ? angle - 180.0 : angle + 180.0), 4, 'f', 3, '0');

    const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, text);

    //    QLineF line(m_pt2, m_pt1);

    if (qFuzzyIsNull(line.length()))
        return;

    const double k = GraphicsView::scaleFactor();

    painter->setBrush(QColor(127, 127, 127, 100));
    painter->setPen(QPen(Qt::green, 0.0)); //1.5 * k));
    // draw rect
    painter->drawRect(rect);
    // draw cross
    const double crossLength = 20.0 * k;
    painter->drawLine(QLineF::fromPolar(crossLength, 0.000).translated(pt1));
    painter->drawLine(QLineF::fromPolar(crossLength, 90.00).translated(pt1));
    painter->drawLine(QLineF::fromPolar(crossLength, 180.0).translated(pt1));
    painter->drawLine(QLineF::fromPolar(crossLength, 270.0).translated(pt1));

    painter->drawLine(QLineF::fromPolar(crossLength, 0.000).translated(pt2));
    painter->drawLine(QLineF::fromPolar(crossLength, 90.00).translated(pt2));
    painter->drawLine(QLineF::fromPolar(crossLength, 180.0).translated(pt2));
    painter->drawLine(QLineF::fromPolar(crossLength, 270.0).translated(pt2));

    // draw arrow
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(QPen(Qt::white, 0.0));
    painter->drawLine(line);
    line.setLength(20.0 * k);
    line.setAngle(angle + 10);
    painter->drawLine(line);
    line.setAngle(angle - 10);
    painter->drawLine(line);
    // draw text
    //painter->setFont(font);
    //painter->drawText(textRect, Qt::AlignLeft, text);
    QPointF pt(pt2);
    if ((pt.x() + textRect.width() * k) > m_rect.right())
        pt.rx() -= textRect.width() * k;
    if ((pt.y() - textRect.height() * k) < m_rect.top())
        pt.ry() += textRect.height() * k;
    painter->translate(pt);
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
}

void Scene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (m_drawPdf)
        return;

    painter->fillRect(rect, Settings::color(Colors::Background));

    //    if (qFuzzyIsNull(itemsBoundingRect().width()) || qFuzzyIsNull(itemsBoundingRect().height()))
    //        return;

    //    painter->save();

    //    //    if (GerberProject::getDrawingType() == GerberProject::RAW)
    //    //        painter->setBrush(QColor(128, 128, 0));
    //    //    else
    //    painter->setBrush(Qt::NoBrush);

    //    painter->setPen(QPen(Qt::white, 0.0));
    //    painter->drawRect(itemsBoundingRect() /*+ QMarginsF(2, 2, 2, 2)*/);
    //    painter->restore();
}

void Scene::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (m_drawPdf)
        return;

    const long k = 10000;
    const double invK = 1.0 / k;

    if (!qFuzzyCompare(m_scale, views().first()->matrix().m11()) || m_rect != rect) {

        m_scale = views().first()->matrix().m11();
        if (qFuzzyIsNull(m_scale))
            return;

        m_rect = rect;

        hGrid.clear();
        vGrid.clear();

        double gridStep = qPow(10.0, qCeil(log10((8.0 / m_scale)))); // 0.1;
        for (long hPos = static_cast<long>(qFloor(rect.left() / gridStep) * gridStep * k),
                  right = static_cast<long>(rect.right() * k),
                  step = static_cast<long>(gridStep * k);
             hPos < right; hPos += step) {
            hGrid[hPos] = 0;
        }
        for (long vPos = static_cast<long>(qFloor(rect.top() / gridStep) * gridStep * k),
                  bottom = static_cast<long>(rect.bottom() * k),
                  step = static_cast<long>(gridStep * k);
             vPos < bottom; vPos += step) {
            vGrid[vPos] = 0;
        }

        gridStep *= 5; // 0.5;
        for (long hPos = static_cast<long>(qFloor(rect.left() / gridStep) * gridStep * k),
                  right = static_cast<long>(rect.right() * k),
                  step = static_cast<long>(gridStep * k);
             hPos < right; hPos += step) {
            hGrid[hPos] = 1;
        }
        for (long vPos = static_cast<long>(qFloor(rect.top() / gridStep) * gridStep * k),
                  bottom = static_cast<long>(rect.bottom() * k),
                  step = static_cast<long>(gridStep * k);
             vPos < bottom; vPos += step) {
            vGrid[vPos] = 1;
        }

        gridStep *= 2; // 1.0;
        for (long hPos = static_cast<long>(qFloor(rect.left() / gridStep) * gridStep * k),
                  right = static_cast<long>(rect.right() * k),
                  step = static_cast<long>(gridStep * k);
             hPos < right; hPos += step) {
            hGrid[hPos] = 2;
        }
        for (long vPos = static_cast<long>(qFloor(rect.top() / gridStep) * gridStep * k),
                  bottom = static_cast<long>(rect.bottom() * k),
                  step = static_cast<long>(gridStep * k);
             vPos < bottom; vPos += step) {
            vGrid[vPos] = 2;
        }
    }

    const QColor color[3]{
        Settings::color(Colors::Grid1),
        Settings::color(Colors::Grid5),
        Settings::color(Colors::Grid10),
    };

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);

    for (int i = 0; i < 3; ++i) {
        painter->setPen(QPen(color[i], 1.0 / m_scale));
        for (long hPos : hGrid.keys(i)) {
            if (hPos)
                painter->drawLine(QLineF(hPos * invK, rect.top(), hPos * invK, rect.bottom()));
        }
        for (long vPos : vGrid.keys(i)) {
            if (vPos)
                painter->drawLine(QLineF(rect.left(), vPos * invK, rect.right(), vPos * invK));
        }
    }

    const double k2 = 0.5 / m_scale;

    painter->setPen(QPen(QColor(255, 0, 0, 100), 0.0 /*1.0 / scale*/));
    painter->drawLine(QLineF(k2, rect.top(), k2, rect.bottom()));
    painter->drawLine(QLineF(rect.left(), -k2, rect.right(), -k2));

    QList<QGraphicsItem*> items = QGraphicsScene::items(m_cross, Qt::IntersectsItemShape, Qt::DescendingOrder, views().first()->transform());
    bool fl = false;
    for (QGraphicsItem* item : items) {
        if (item && item->type() != RulerType && item->type() != BridgeType && item->flags() & QGraphicsItem::ItemIsSelectable) {
            fl = true;
            break;
        }
    }
    if (fl)
        painter->setPen(QPen(QColor(255, 000, 000, 150), 0.0 /*1.0 / scale*/));
    else
        painter->setPen(QPen(QColor(255, 255, 255, 150), 0.0 /*1.0 / scale*/));

    painter->drawLine(QLineF(m_cross.x(), rect.top(), m_cross.x(), rect.bottom()));
    painter->drawLine(QLineF(rect.left(), m_cross.y(), rect.right(), m_cross.y()));

    if (!m_cross2.isNull())
        drawRuller(painter);

    painter->restore();
}
