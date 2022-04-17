// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "scene.h"

#include "graphicsview.h"
#include "project.h"
#include "settings.h"
#include <graphicsitem.h>

#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QTime>
#include <QtMath>

// QTime m_time;
// int m_time2;
// int m_frameCount = 0;
// int m_frameCount2;

Scene::Scene(QObject* parent)
    : QGraphicsScene(parent) {
    App::setScene(this);
    double size = 1000.0; // 4 sqare meters
    setSceneRect(-size, -size, +size * 2, +size * 2);

    startTimer(1000);
}

Scene::~Scene() {
    App::setScene(nullptr);
}

void Scene::RenderPdf() {
    QString curFile = QFileDialog::getSaveFileName(nullptr, tr("Save PDF file"), "File", tr("File(*.pdf)"));
    if (curFile.isEmpty())
        return;

    m_drawPdf = true;

    QRectF rect;

    for (QGraphicsItem* item : items())
        if (item->isVisible() && !item->boundingRect().isNull())
            rect |= item->boundingRect();

    // QRectF rect(QGraphicsScene::itemsBoundingRect());

    QPdfWriter pdfWriter(curFile);
#if QT_VERSION > QT_VERSION_CHECK(5, 99, 99)
    pdfWriter.setPageSize(QPageSize(rect.size(), QPageSize::Millimeter));
    pdfWriter.setPageMargins({ 0, 0, 0, 0 });
#else
    QSizeF size(rect.size());
    pdfWriter.setPageSizeMM(size);
    pdfWriter.setMargins({ 0, 0, 0, 0 });
#endif
    pdfWriter.setResolution(1000000);

    QPainter painter(&pdfWriter);
    painter.setTransform(QTransform().scale(1.0, -1.0));
    painter.translate(0, -(pdfWriter.resolution() / 25.4) * rect.size().height());
    render(&painter,
        QRectF(0, 0, pdfWriter.width(), pdfWriter.height()),
        rect, Qt::IgnoreAspectRatio);

    m_drawPdf = false;
}

QRectF Scene::itemsBoundingRect() {
    m_drawPdf = true;
    QRectF rect(QGraphicsScene::itemsBoundingRect());
    m_drawPdf = false;
    return rect;
}

QRectF Scene::getSelectedBoundingRect() {
    auto selectedItems(App::scene()->selectedItems());

    if (selectedItems.isEmpty())
        return {};

    m_boundingRect = true;
    QRectF rect(selectedItems.takeFirst()->boundingRect());
    for (auto gi : selectedItems)
        rect = rect.united(gi->boundingRect());
    m_boundingRect = true;

    if (!rect.isEmpty())
        App::project()->setWorckRect(rect);

    return rect;
}

void Scene::setCross1(const QPointF& cross) {
    m_cross1 = cross;
    update();
}

void Scene::setCross2(const QPointF& cross2) {
    m_cross2 = cross2;
}

void Scene::setDrawRuller(bool drawRuller) {
    m_drawRuller = drawRuller;
    update();
}

void Scene::drawRuller(QPainter* painter) {
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
    const QString text = QString(App::settings().inch() ? "  ∆X: %1 in\n"
                                                          "  ∆Y: %2 in\n"
                                                          "  ∆/: %3 in\n"
                                                          "  Area: %4 in²\n"
                                                          "  Angle: %5°"
                                                        : "  ∆X: %1 mm\n"
                                                          "  ∆Y: %2 mm\n"
                                                          "  ∆/: %3 mm\n"
                                                          "  Area: %4 mm²\n"
                                                          "  Angle: %5°")
                             .arg(rect.width() / (App::settings().inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg(rect.height() / (App::settings().inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg(length / (App::settings().inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg((rect.width() / (App::settings().inch() ? 25.4 : 1.0))
                                     * (rect.height() / (App::settings().inch() ? 25.4 : 1.0)),
                                 4, 'f', 3, '0')
                             .arg(360.0 - (angle > 180.0 ? angle - 180.0 : angle + 180.0), 4, 'f', 3, '0');

    const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, text);

    if (qFuzzyIsNull(line.length()))
        return;

    const double scaleFactor = App::graphicsView()->scaleFactor();
    painter->save();
    painter->setBrush(QColor(127, 127, 127, 100));
    painter->setPen(QPen(Qt::green, 0.0));
    {
        // draw rect
        painter->drawRect(rect);
        const double crossLength = 20.0 * scaleFactor;
        // draw cross pt1
        painter->drawLine(pt1, pt1 + QPointF(0, crossLength));
        painter->drawLine(pt1, pt1 + QPointF(crossLength, 0));
        painter->drawLine(pt1, pt1 - QPointF(0, crossLength));
        painter->drawLine(pt1, pt1 - QPointF(crossLength, 0));
        // draw cross pt1
        painter->drawLine(pt2, pt2 + QPointF(0, crossLength));
        painter->drawLine(pt2, pt2 + QPointF(crossLength, 0));
        painter->drawLine(pt2, pt2 - QPointF(0, crossLength));
        painter->drawLine(pt2, pt2 - QPointF(crossLength, 0));
    }

    { // draw arrow
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(Qt::white, 0.0));
        painter->drawLine(line);
        line.setLength(20.0 * scaleFactor);
        line.setAngle(angle + 10);
        painter->drawLine(line);
        line.setAngle(angle - 10);
        painter->drawLine(line);
    }
    // draw text
    // painter->setFont(font);
    // painter->drawText(textRect, Qt::AlignLeft, text);
    QPointF pt(pt2);
    if ((pt.x() + textRect.width() * scaleFactor) > m_rect.right())
        pt.rx() -= textRect.width() * scaleFactor;
    if ((pt.y() - textRect.height() * scaleFactor) < m_rect.top())
        pt.ry() += textRect.height() * scaleFactor;
    painter->translate(pt);
    painter->scale(scaleFactor, -scaleFactor);
    int i = 0;
    for (const QString& txt : text.split('\n')) {
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

void Scene::drawBackground(QPainter* painter, const QRectF& rect) {
    if (m_drawPdf)
        return;

    painter->fillRect(rect, App::settings().guiColor(GuiColors::Background));
}

void Scene::drawForeground(QPainter* painter, const QRectF& rect) {
    if (m_drawPdf)
        return;

    ++fpsCtr;

    { // draw grid
        const long upScale = 100000;
        const long forLimit = 1000;
        const double downScale = 1.0 / upScale;
        static bool in = App::settings().inch();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        if (!qFuzzyCompare(m_scale, views().first()->matrix().m11()) || m_rect != rect || in != App::settings().inch()) {
            in = App::settings().inch();
            m_scale = views().first()->matrix().m11();
#else
        if (!qFuzzyCompare(m_scale, views().first()->transform().m11()) || m_rect != rect || in != App::settings().inch()) {
            in = App::settings().inch();
            m_scale = views().first()->transform().m11();
#endif
            if (qFuzzyIsNull(m_scale))
                return;

            m_rect = rect;
            hGrid.clear();
            vGrid.clear();

            // Grid Step 0.1
            double gridStep = App::settings().gridStep(m_scale);
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
            App::settings().guiColor(GuiColors::Grid1),
            App::settings().guiColor(GuiColors::Grid5),
            App::settings().guiColor(GuiColors::Grid10),
        };

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);
        QElapsedTimer t;
        t.start();
        const double k2 = 0.5 / m_scale;
        // painter->setCompositionMode(QPainter::CompositionMode_Lighten);

        for (int i = 0; i < 3; ++i) {
            painter->setPen(QPen(color[i], 0.0));
            for (long hPos : hGrid.keys(i)) {
                if (hPos)
                    painter->drawLine(QLineF(hPos * downScale + k2, rect.top(), hPos * downScale + k2, rect.bottom()));
            }
            for (long vPos : vGrid.keys(i)) {
                if (vPos)
                    painter->drawLine(QLineF(rect.left(), vPos * downScale + k2, rect.right(), vPos * downScale + k2));
            }
        }

        // zero cross
        painter->setPen(QPen(QColor(255, 0, 0, 100), 0.0));
        painter->drawLine(QLineF(k2, rect.top(), k2, rect.bottom()));
        painter->drawLine(QLineF(rect.left(), -k2, rect.right(), -k2));
    }

    if (1) { // screen mouse cross
        QList<QGraphicsItem*> items = QGraphicsScene::items(m_cross1, Qt::IntersectsItemShape, Qt::DescendingOrder, views().first()->transform());
        bool fl = false;
        for (QGraphicsItem* item : items) {
            if (item && static_cast<GiType>(item->type()) != GiType::Bridge && item->flags() & QGraphicsItem::ItemIsSelectable) {
                fl = true;
                break;
            }
        }
        if (fl)
            painter->setPen(QPen(QColor(255, 000, 000, 255), 0.0));
        else {
            QColor c(App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF);
            //            c.setAlpha(150);
            painter->setPen(QPen(c, 0.0));
        }

        painter->drawLine(QLineF(m_cross1.x(), rect.top(), m_cross1.x(), rect.bottom()));
        painter->drawLine(QLineF(rect.left(), m_cross1.y(), rect.right(), m_cross1.y()));
    }

    if (m_drawRuller)
        drawRuller(painter);

    //    if (0) {
    //        if (m_frameCount == 0) {
    //            m_time.start();
    //            m_time2 = m_time.elapsed() + 1000;

    //        } else {
    //            if (m_time.elapsed() > m_time2) {

    //                m_time2 = m_time.elapsed() + 1000;
    //                m_frameCount2 = m_frameCount;
    //                m_frameCount = 0;
    //            }

    //            painter->setRenderHint(QPainter::Antialiasing, true);
    //            QString str(QString("FPS %1").arg(m_frameCount2));
    //            painter->translate(m_rect.center());
    //            const double scaleFactor = App::graphicsView()->scaleFactor();
    //            painter->scale(scaleFactor, -scaleFactor);
    //            QFont f;
    //            f.setPixelSize(100);
    //            QPainterPath path;
    //            path.addText(QPointF(), f, str);
    //            painter->setPen(QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //            painter->setBrush(Qt::NoBrush);
    //            painter->drawPath(path);
    //            painter->setPen(Qt::NoPen);
    //            painter->setBrush(Qt::white);
    //            painter->drawPath(path);
    //        }
    //        m_frameCount++;
    //    }

    { // NOTE FPS counter
        painter->setRenderHint(QPainter::Antialiasing, true);

        const double scaleFactor = App::graphicsView()->scaleFactor();
        painter->translate(rect.bottomLeft());
        painter->scale(scaleFactor, -scaleFactor);

        QPainterPath path;

        auto txt { QString("FPS: %1").arg(currentFps) };

        QFont font;
        font.setPixelSize(16);
        font.setWeight(QFont::Thin);

        const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, txt);
        path.addText(textRect.topLeft() + QPointF(textRect.left(), textRect.height()), font, txt);

        painter->setPen(QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::black);
        painter->drawPath(path);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawPath(path);
    }

    painter->restore();
}

void Scene::timerEvent(QTimerEvent* event) {
    currentFps = fpsCtr;
    fpsCtr = 0;
}
