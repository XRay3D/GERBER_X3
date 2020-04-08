#include "scene.h"
#include "graphicsview.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QtMath>
#include <gi/graphicsitem.h>
#include <settings.h>

Scene* Scene::m_instance = nullptr;

Scene::Scene(QObject* parent)
    : QGraphicsScene(parent)
{
    if (m_instance) {
        QMessageBox::critical(nullptr, "Err", "You cannot create class Scene more than 2 times!!!");
        exit(1);
    }
    m_instance = this;
}

Scene::~Scene() { m_instance = nullptr; }

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
    if (m_instance)
        return m_instance->m_drawPdf;
    return false;
}

QList<QGraphicsItem*> Scene::selectedItems()
{
    if (m_instance)
        return m_instance->QGraphicsScene::selectedItems();
    return {};
}

void Scene::addItem(QGraphicsItem* item)
{
    if (m_instance)
        m_instance->QGraphicsScene::addItem(item);
}

QList<QGraphicsItem*> Scene::items(Qt::SortOrder order)
{
    if (m_instance)
        return m_instance->QGraphicsScene::items(order);
    return {};
}

void Scene::update()
{
    if (m_instance)
        m_instance->QGraphicsScene::update();
}

void Scene::setCross1(const QPointF& cross)
{
    m_cross1 = cross;
    update();
}

void Scene::setCross2(const QPointF& cross2)
{
    m_cross2 = cross2;
}

void Scene::setDrawRuller(bool drawRuller)
{
    m_drawRuller = drawRuller;
    update();
}

void Scene::drawRuller(QPainter* painter)
{
    const QPointF pt1(m_cross2);
    const QPointF pt2(m_cross1);
    QLineF line(pt2, pt1);
    const QRectF rect(
        QPointF(qMin(pt1.x(), pt2.x()), qMin(pt1.y(), pt2.y())),
        QPointF(qMax(pt1.x(), pt2.x()), qMax(pt1.y(), pt2.y())));
    const double length = line.length();
    const double angle = line.angle();

    QFont font;
    font.setPixelSize(16);
    const QString text = QString(GlobalSettings::inch() ? "  ∆X = %1 in\n"
                                                          "  ∆Y = %2 in\n"
                                                          "  ∆ / = %3 in\n"
                                                          "  %4°"
                                                        : "  ∆X = %1 mm\n"
                                                          "  ∆Y = %2 mm\n"
                                                          "  ∆ / = %3 mm\n"
                                                          "  %4°")
                             .arg(rect.width() / (GlobalSettings::inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg(rect.height() / (GlobalSettings::inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg(length / (GlobalSettings::inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg(360.0 - (angle > 180.0 ? angle - 180.0 : angle + 180.0), 4, 'f', 3, '0');

    const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, text);

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

    painter->fillRect(rect, GlobalSettings::guiColor(Colors::Background));
}

void Scene::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (m_drawPdf)
        return;

    { // draw grid
        const long upScale = 1000000;
        const long forLimit = 10000;
        const double downScale = 1.0 / upScale;
        static bool in = GlobalSettings::inch();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        if (!qFuzzyCompare(m_scale, views().first()->matrix().m11()) || m_rect != rect || in != GlobalSettings::inch()) {
            in = GlobalSettings::inch();
            m_scale = views().first()->matrix().m11();
#else
        if (!qFuzzyCompare(m_scale, views().first()->transform().m11()) || m_rect != rect || in != GlobalSettings::inch()) {
            in = GlobalSettings::inch();
            m_scale = views().first()->transform().m11();
#endif
            if (qFuzzyIsNull(m_scale))
                return;

            m_rect = rect;
            hGrid.clear();
            vGrid.clear();

            // Grid Step 0.1
            double gridStep = GlobalSettings::gridStep(m_scale);
            for (long hPos = static_cast<long>(qFloor(rect.left() / gridStep) * gridStep * upScale),
                      right = static_cast<long>(rect.right() * upScale),
                      step = static_cast<long>(gridStep * upScale), nlp = 0;
                 hPos < right && ++nlp < forLimit; hPos += step) {
                hGrid[hPos] = 0;
            }
            for (long vPos = static_cast<long>(qFloor(rect.top() / gridStep) * gridStep * upScale),
                      bottom = static_cast<long>(rect.bottom() * upScale),
                      step = static_cast<long>(gridStep * upScale), nlp = 0;
                 vPos < bottom && ++nlp < forLimit; vPos += step) {
                vGrid[vPos] = 0;
            }
            // Grid Step  0.5
            gridStep *= 5;
            for (long hPos = static_cast<long>(qFloor(rect.left() / gridStep) * gridStep * upScale),
                      right = static_cast<long>(rect.right() * upScale),
                      step = static_cast<long>(gridStep * upScale), nlp = 0;
                 hPos < right && ++nlp < forLimit; hPos += step) {
                hGrid[hPos] = 1;
            }
            for (long vPos = static_cast<long>(qFloor(rect.top() / gridStep) * gridStep * upScale),
                      bottom = static_cast<long>(rect.bottom() * upScale),
                      step = static_cast<long>(gridStep * upScale), nlp = 0;
                 vPos < bottom && ++nlp < forLimit; vPos += step) {
                vGrid[vPos] = 1;
            }
            // Grid Step  1.0
            gridStep *= 2;
            for (long hPos = static_cast<long>(qFloor(rect.left() / gridStep) * gridStep * upScale),
                      right = static_cast<long>(rect.right() * upScale),
                      step = static_cast<long>(gridStep * upScale), nlp = 0;
                 hPos < right && ++nlp < forLimit; hPos += step) {
                hGrid[hPos] = 2;
            }
            for (long vPos = static_cast<long>(qFloor(rect.top() / gridStep) * gridStep * upScale),
                      bottom = static_cast<long>(rect.bottom() * upScale),
                      step = static_cast<long>(gridStep * upScale), nlp = 0;
                 vPos < bottom && ++nlp < forLimit; vPos += step) {
                vGrid[vPos] = 2;
            }
        }

        const QColor color[] {
            GlobalSettings::guiColor(Colors::Grid1),
            GlobalSettings::guiColor(Colors::Grid5),
            GlobalSettings::guiColor(Colors::Grid10),
        };

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);

        for (int i = 0; i < 3; ++i) {
            painter->setPen(QPen(color[i], 1.0 / m_scale));
            for (long hPos : hGrid.keys(i)) {
                if (hPos)
                    painter->drawLine(QLineF(hPos * downScale, rect.top(), hPos * downScale, rect.bottom()));
            }
            for (long vPos : vGrid.keys(i)) {
                if (vPos)
                    painter->drawLine(QLineF(rect.left(), vPos * downScale, rect.right(), vPos * downScale));
            }
        }

        const double k2 = 0.5 / m_scale;

        painter->setPen(QPen(QColor(255, 0, 0, 100), 0.0 /*1.0 / scale*/));
        painter->drawLine(QLineF(k2, rect.top(), k2, rect.bottom()));
        painter->drawLine(QLineF(rect.left(), -k2, rect.right(), -k2));
    }

    { // screen mouse cross
        QList<QGraphicsItem*> items = QGraphicsScene::items(m_cross1, Qt::IntersectsItemShape, Qt::DescendingOrder, views().first()->transform());
        bool fl = false;
        for (QGraphicsItem* item : items) {
            if (item && item->type() != GiBridge && item->flags() & QGraphicsItem::ItemIsSelectable) {
                fl = true;
                break;
            }
        }
        if (fl)
            painter->setPen(QPen(QColor(255, 000, 000, 150), 0.0 /*1.0 / scale*/));
        else
            painter->setPen(QPen(QColor(255, 255, 000, 150), 0.0 /*1.0 / scale*/));

        painter->drawLine(QLineF(m_cross1.x(), rect.top(), m_cross1.x(), rect.bottom()));
        painter->drawLine(QLineF(rect.left(), m_cross1.y(), rect.right(), m_cross1.y()));
    }

    if (m_drawRuller)
        drawRuller(painter);

    painter->restore();
}
