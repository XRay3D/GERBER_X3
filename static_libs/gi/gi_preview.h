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

#include "gi.h"
#include "qparallelanimationgroup.h"
#include "tool.h"

namespace Gi {

class AbstractPreview : public QGraphicsObject {
    friend class Node;

    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor NOTIFY colorChanged FINAL)

    QColor bodyColor() { return bodyColor_; }
    void setBodyColor(const QColor& c) { bodyColor_ = c, colorChanged(); }
    QColor pathColor() { return pathColor_; }
    void setPathColor(const QColor& c) { pathColor_ = c, colorChanged(); }

    QParallelAnimationGroup propAnimGr;
    QPropertyAnimation propAnimBr;
    QPropertyAnimation propAnimPn;

signals:
    void colorChanged();

public:
    AbstractPreview();
    ~AbstractPreview() override = default;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    //////////////////////////////////////////
    int type() const override;
    double sourceDiameter() const;
    // Признак "участвует в построении G-кода" для конкретного экземпляра превью
    // (в отличие от Row::useForCalc, который включает/выключает сразу всю строку).
    bool isUsed() const { return +(colorState & ColorState::Used); }
    virtual Tool::ID toolId() const = 0;
    virtual void updateTool() = 0;
    // virtual Paths paths() const          = 0;
    virtual bool fit(double depth) const = 0;

    void changeColor();

protected:
    QPainterPath sourcePath_;
    QPainterPath toolPath_;

    // Кэш пути инструмента для ветки toolPath_.isEmpty(): Tool::path() строит
    // QPainterPath целиком, а от кадра к кадру меняются только id и позиция.
    QPainterPath toolPathCache_;
    Tool::ID toolPathCacheId_{Tool::ID::Null};
    QPointF toolPathCachePos_;

    double sourceDiameter_{};

    QColor bodyColor_;
    QColor pathColor_;

    struct StateColors {
        QColor body; // цвет тела превью (bodyColor_)
        QColor path; // цвет контура инструмента (pathColor_)
    };

    enum class ColorState : unsigned {
        Default,
        Hovered = 1,
        Selected = 2,
        Used = 4,
        Tool = 8,
    } colorState{ColorState::Used};
    using CS = ColorState;

    static constexpr int dark = 180;  // обычная (невыделенная, ненаведённая) насыщенность
    static constexpr int light = 255; // Selected — самое яркое состояние
    static constexpr int hover = 120; // любое наведение мышью — тусклее Selected (и обычного состояния тоже)

    // Именованные цвета контура/пути (path) превью.
    static inline constexpr QColor pathNone{255, 255, 255, 255};  // инструмент не назначен либо не используется — контур не рисуем
    static inline constexpr QColor pathIdle{128, 128, 128, dark}; // инструмент назначен, но не используется (Tool без Used)
    static inline constexpr QColor pathTool{255, 255, 255, dark}; // инструмент назначен и используется — контур инструмента
    // Таблица покрывает все 16 комбинаций битов ColorState (Hovered|Selected|Used|Tool) —
    // индекс это само значение colorState, поэтому changeColor() обходится без условий.
    // Приоритет тела: Selected > Used > Default. Путь зависит только от Used и Tool.
    inline static constexpr std::array colors{
        StateColors{QColor{128, 128, 128, 128}, pathNone}, //  0 ----
        StateColors{QColor{192, 192, 192, 128}, pathNone}, //  1 ---H Hovered
        StateColors{QColor{128, 128, 128, 255}, pathNone}, //  2 --S- Selected
        StateColors{QColor{192, 192, 192, 255}, pathNone}, //  3 --SH Selected+Hovered
        StateColors{QColor{128, 128, 128, 128}, pathNone}, //  4 -U-- Used
        StateColors{QColor{128, 128, 128, 128}, pathNone}, //  5 -U-H Used+Hovered
        StateColors{QColor{128, 128, 128, 255}, pathNone}, //  6 -US- Used+Selected (Selected приоритетнее)
        StateColors{QColor{128, 128, 128, 255}, pathNone}, //  7 -USH Used+Selected+Hovered
        StateColors{QColor{128, 0, 0, 128},     pathIdle}, //  8 T--- Tool
        StateColors{QColor{128, 0, 0, 128},     pathIdle}, //  9 T--H Tool+HovedarkRed
        StateColors{QColor{128, 0, 0, 255},     pathIdle}, // 10 T-S- Tool+Selected
        StateColors{QColor{128, 0, 0, 255},     pathIdle}, // 11 T-SH Tool+Selected+Hovered
        StateColors{QColor{255, 0, 0, 128},     pathTool}, // 12 TU-- Tool+Used
        StateColors{QColor{255, 0, 0, 128},     pathTool}, // 13 TU-H Tool+Used+Hovered
        StateColors{QColor{255, 0, 0, 255},     pathTool}, // 14 TUS- Tool+Used+Selected
        StateColors{QColor{255, 64, 64, 255},   pathTool}, // 15 TUSH Tool+Used+Selected+Hovered
    };

    int i = (128 + 64);

    // QGraphicsItem interface
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

} // namespace Gi
