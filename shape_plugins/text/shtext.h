/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "shape.h"
#include <QJsonObject>
#include <graphicsitem.h>
#include <interfaces/plugintypes.h>
#include <interfaces/shapepluginin.h>

class ShTextDialog;

namespace Shapes {
class Text final : public Shape {
    friend ShTextDialog;
    friend Shape;

public:
    explicit Text(QPointF pt1);
    explicit Text() { }
    ~Text() = default;

    enum {
        BotCenter = 1,
        BotLeft,
        BotRight,
        Center,
        CenterLeft,
        CenterRight,
        TopCenter,
        TopLeft,
        TopRight,
    };

    struct InternalData {
        friend QDataStream& operator<<(QDataStream& stream, const InternalData& d);
        friend QDataStream& operator>>(QDataStream& stream, InternalData& d);
        InternalData()
            : font("")
            , text("Text")
            , side(Top)
            , angle(0.0)
            , height(10.0)
            , xy(0.0)
            , handleAlign(BotLeft)
        {
        }
        QString font;
        QString text;
        Side side;
        double angle;
        double height;
        double xy;
        int handleAlign;
    };

    // QGraphicsItem interface
    int type() const override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    QPainterPath shape() const override; // Shape interface

    // Shape interface
    void redraw() override;
    QString name() const override;
    QIcon icon() const override;

    QString text() const;
    void setText(const QString& value);

    Side side() const;
    void setSide(const Side& side);
    // Shape interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTree::View* tv) const override;

protected:
    // Shape interface
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

private:
    InternalData iData;
    InternalData iDataCopy;
    inline static InternalData lastUsedIData;
    static void saveIData();
    static InternalData loadIData();

    void save();
    void restore();
    void ok();
};

class PluginText : public QObject, public ShapePluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "text.json")
    Q_INTERFACES(ShapePluginInterface)

    Text* shape = nullptr;

public:
    PluginText();
    virtual ~PluginText() override;

    // ShapePluginTextInterface interface
public:
    QObject* getObject() override;
    int type() const override;
    QJsonObject info() const override;
    QIcon icon() const override;
    Shape* createShape() override;
    Shape* createShape(const QPointF& point) override;
    bool addShapePoint(const QPointF& value) override;
    void updateShape(const QPointF& value) override;
    void finalizeShape() override;

signals:
    void actionUncheck(bool = false) override;
};

}
