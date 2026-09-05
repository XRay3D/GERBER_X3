/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gc_types.h"
#include "gi.h"

namespace Gi {

// Мост (таб): маркер на исходном контуре, где деталь остаётся прихваченной к
// заготовке. Элемент живёт только на сцене: в расчёт УП мосты уходят своими
// центрами (Profile::Form::computePaths -> gcp.supportCurvess), а круги реза
// по ним строит Profile::File сам -- их радиус зависит от припуска чистового
// прохода, известного лишь там.
class Bridge final : public Item {

public:
    explicit Bridge();
    ~Bridge() override { moveBrPtr() = nullptr; }

    // QGraphicsItem interface
    QRectF boundingRect() const override { return pPath.boundingRect(); }
    QPainterPath shape() const override { return pPath; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // Item interface
    Geo::Polylines curves(int alternate = {}) const override;
    void updateColors() override { }

    bool ok() const;
    void update();
    QPointF snapedPos(const QPointF& pos);

    // Inline-статические члены класса живут ОТДЕЛЬНОЙ копией в каждом
    // модуле (плагин / ядро ggcore) на Windows: писать их из кода плагина --
    // писать не в ту копию, что читает реализация в ядре. Поэтому наружу
    // они отдаются ссылками через функции, определённые в gc_gi_bridge.cpp
    // (одна копия в ggcore на весь процесс).
    static Bridge*& moveBrPtr();
    static double& lenght();
    static double& toolDiam();
    static GCode::SideOfMilling& millingSide();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    double angle_{};

    QPainterPath pPath;
    QPainterPath cutoff;

    QPointF lastPos;

    bool ok_{};
};

} // namespace Gi
