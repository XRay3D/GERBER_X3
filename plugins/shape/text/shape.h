/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "editor.h"
#include "gi.h"
#include "shape.h"
#include "shapepluginin.h"

class ShTextDialog;

namespace ShTxt {

class Shape final : public Shapes::AbstractShape {
    friend ShTextDialog;
    friend AbstractShape;
    friend class Editor;

public:
    explicit Shape(Shapes::Plugin* plugin, QPointF pt1 = {});
    ~Shape() override = default;

    enum {
        // clang-format off
        BotCenter   = Qt::AlignBottom | Qt::AlignHCenter,
        BotLeft     = Qt::AlignBottom | Qt::AlignLeft,
        BotRight    = Qt::AlignBottom | Qt::AlignRight,

        Center      = Qt::AlignHCenter | Qt::AlignVCenter,
        CenterLeft  = Qt::AlignHCenter | Qt::AlignLeft,
        CenterRight = Qt::AlignHCenter | Qt::AlignRight,

        TopCenter   = Qt::AlignTop | Qt::AlignHCenter,
        TopLeft     = Qt::AlignTop | Qt::AlignLeft,
        TopRight    = Qt::AlignTop | Qt::AlignRight,
        // clang-format on
    };

    struct ShapeData {
        QFont font{};
        QString text{"Shape"};
        double angle{0.0};
        double height{10.0};
        double xy{100.0};
        Side side{Top};
        int handleAlign{BotLeft};
    };

    // QGraphicsItem interface
    int type() const override { return Gi::Type::ShText; }

    // AbstractShape interface
    void redraw() override;
    QString name() const override;
    QIcon icon() const override;
    void setPt(const QPointF& point) override;

    QString text() const;
    void setText(const QString& value);

    Side side() const;
    void setSide(const Side& side);

    // AbstractShape interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;

protected:
    // AbstractShape interface
    void write(QDataStream& stream) const override;
    void readAndInit(QDataStream& stream) override;

private:
    ShapeData iData;
    ShapeData iDataCopy;
    void saveIData();
    ShapeData loadIData();

    void save();
    void restore();
    void ok();
};

class Plugin : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "description.json")
    Q_INTERFACES(Shapes::Plugin)
    Editor editor_{this};

public:
    // Shapes::Plugin interface
    uint32_t type() const override { return Gi::Type::ShText; }
    QIcon icon() const override { return QIcon::fromTheme("draw-text"); }
    Shapes::AbstractShape* createShape(const QPointF& point = {}) override {
        auto shape = new Shape{this, point};
        editor_.add(shape);
        return shape;
    }
    Editor* editor() override { return &editor_; };
};

} // namespace ShTxt
