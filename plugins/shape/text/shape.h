/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
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
    explicit Shape(QPointF pt1 = {});
    ~Shape() override;

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

    struct InternalData {
        QString font{};
        QString text{"Shape"};
        Side side{Top};
        double angle{0.0};
        double height{10.0};
        double xy{100.0};
        int handleAlign{BotLeft};
    };

    // QGraphicsItem interface
    int type() const override { return Gi::Type::ShText; }
    //    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    QPainterPath shape() const override; // AbstractShape interface

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

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
    //    void menu(QMenu& menu, FileTree::View* tv) override;

    Editor* editor{};

protected:
    // AbstractShape interface
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

private:
    InternalData iData;
    InternalData iDataCopy;
    void saveIData();
    InternalData loadIData();

    void save();
    void restore();
    void ok();
};

class Plugin : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "text.json")
    Q_INTERFACES(Shapes::Plugin)
    mutable Editor editor_{this};

public:
    // Shapes::Plugin interface
    uint32_t type() const override { return Gi::Type::ShText; }
    QIcon icon() const override { return QIcon::fromTheme("draw-text"); }
    Shapes::AbstractShape* createShape(const QPointF& point = {}) const override {
        auto shape = new Shape(point);
        editor_.addShape(shape);
        return shape;
    }
    QWidget* editor() override { return &editor_; };
};

} // namespace ShTxt
