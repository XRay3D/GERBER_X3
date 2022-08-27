// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
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

#include "gi.h"
#include "graphicsview.h"
#include "project.h"
#include "settings.h"

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
#include <utils.h>

// QTime time_;
// int time2_;
// int frameCount_ = 0;
// int frameCount2_;

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

void Scene::renderPdf() {
    QString curFile = QFileDialog::getSaveFileName(nullptr, tr("Save PDF file"), "File", tr("File(*.pdf)"));
    if (curFile.isEmpty())
        return;

    ScopedTrue sTrue(drawPdf_);

    QRectF rect;

    for (QGraphicsItem* item : items())
        if (item->isVisible() && !item->boundingRect().isNull())
            rect |= item->boundingRect();

    // QRectF rect(QGraphicsScene::itemsBoundingRect());

    QPdfWriter pdfWriter(curFile);
    pdfWriter.setPageSize(QPageSize(rect.size(), QPageSize::Millimeter));
    pdfWriter.setPageMargins({0, 0, 0, 0});
    pdfWriter.setResolution(1000000);

    QPainter painter(&pdfWriter);
    painter.setTransform(QTransform().scale(1.0, -1.0));
    painter.translate(0, -(pdfWriter.resolution() / 25.4) * rect.size().height());
    render(&painter,
        QRectF(0, 0, pdfWriter.width(), pdfWriter.height()),
        rect, Qt::IgnoreAspectRatio);
}

QRectF Scene::itemsBoundingRect() {
    ScopedTrue sTrue(drawPdf_);
    QRectF rect(QGraphicsScene::itemsBoundingRect());
    return rect;
}

QRectF Scene::getSelectedBoundingRect() {
    auto selectedItems(App::scene()->selectedItems());

    if (selectedItems.isEmpty())
        return {};

    QRectF rect;

    {
        ScopedTrue sTrue(boundingRect_);
        rect = selectedItems.front()->boundingRect();
        for (auto gi : selectedItems)
            rect = rect.united(gi->boundingRect());
    }

    if (!rect.isEmpty())
        App::project()->setWorckRect(rect);

    return rect;
}

void Scene::setCross1(const QPointF& cross) {
    cross1 = cross;
    update();
}

void Scene::setCross2(const QPointF& cross) {
    cross2 = cross;
}

void Scene::setDrawRuller(bool drawRuller) {
    drawRuller = drawRuller;
    update();
}

void Scene::drawRuller(QPainter* painter) {
    const QPointF pt1(cross2);
    const QPointF pt2(cross1);
    QLineF line(pt2, pt1);
    const QRectF rect(
        QPointF(qMin(pt1.x(), pt2.x()), qMin(pt1.y(), pt2.y())),
        QPointF(std::max(pt1.x(), pt2.x()), std::max(pt1.y(), pt2.y())));
    const double length = line.length();
    const double angle = line.angle();

    QFont font;
    font.setPixelSize(16);
    const QString text = QString(App::settings().inch() ? "  ∆X: %1 in\n"
                                                          "  ∆Y: %2 in\n"
                                                          "  ∆/: %3 in\n"
                                                          "  Area: %4 in²\n"
                                                          "  Angle: %5°" :
                                                          "  ∆X: %1 mm\n"
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
    if ((pt.x() + textRect.width() * scaleFactor) > rect.right())
        pt.rx() -= textRect.width() * scaleFactor;
    if ((pt.y() - textRect.height() * scaleFactor) < rect.top())
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
    if (drawPdf_)
        return;

    painter->fillRect(rect, App::settings().guiColor(GuiColors::Background));
}

void Scene::drawForeground(QPainter* painter, const QRectF& rect) {
    if (drawPdf_)
        return;

    ++fpsCtr;

    { // draw grid
        const long upScale = 100000;
        const long forLimit = 1000;
        const double downScale = 1.0 / upScale;
        static bool in = App::settings().inch();

        if (!qFuzzyCompare(scale, views().first()->transform().m11()) || lastRect != rect || in != App::settings().inch()) {
            in = App::settings().inch();
            scale = views().first()->transform().m11();
            if (qFuzzyIsNull(scale))
                return;

            lastRect = rect;

            hGrid.clear();
            vGrid.clear();

            // Grid Step 0.1
            double gridStep = App::settings().gridStep(scale);
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
        const double k2 = 0.5 / scale;
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
        QList<QGraphicsItem*> items = QGraphicsScene::items(cross1, Qt::IntersectsItemShape, Qt::DescendingOrder, views().first()->transform());
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

        painter->drawLine(QLineF(cross1.x(), rect.top(), cross1.x(), rect.bottom()));
        painter->drawLine(QLineF(rect.left(), cross1.y(), rect.right(), cross1.y()));
    }

    if (drawRuller_)
        drawRuller(painter);

    //    if (0) {
    //        if (frameCount_ == 0) {
    //            time_.start();
    //            time2_ = time_.elapsed() + 1000;

    //        } else {
    //            if (time_.elapsed() > time2_) {

    //                time2_ = time_.elapsed() + 1000;
    //                frameCount2_ = frameCount_;
    //                frameCount_ = 0;
    //            }

    //            painter->setRenderHint(QPainter::Antialiasing, true);
    //            QString str(QString("FPS %1").arg(frameCount2_));
    //            painter->translate(rect_.center());
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
    //        frameCount_++;
    //    }

    { // NOTE FPS counter
        painter->setRenderHint(QPainter::Antialiasing, true);

        const double scaleFactor = App::graphicsView()->scaleFactor();
        painter->translate(rect.bottomLeft());
        painter->scale(scaleFactor, -scaleFactor);

        QPainterPath path;

        auto txt {QString("FPS: %1").arg(currentFps)};

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
