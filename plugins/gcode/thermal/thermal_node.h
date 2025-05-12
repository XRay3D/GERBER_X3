/*******************************************************************************
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

#include "thermal_previewitem.h"

#include "thermal_vars.h"

#include <QIcon>
#include <QModelIndex>
#include <memory>

namespace Thermal {

class Model;
class Node;

// class NodeI {
// public:
//     virtual ~NodeI() { }
//     virtual bool isChecked() const = 0;
//     virtual void disable() = 0;
//     virtual void enable() = 0;
//     virtual QModelIndex index(int column = 0) const = 0;

//    virtual double angle() const = 0;
//    virtual double tickness() const = 0;
//    virtual int count() const = 0;
//};

#define OVERRIDE /**/

class Node final /*: public NodeI*/ {
public:
    explicit Node(const QIcon& icon, const QString& name, const ThParam& par, const Point& pos, AbstractThermPrGi* item, Model* model);
    explicit Node(const QIcon& icon, const QString& name, const ThParam& par, Model* model);
    explicit Node(Model* model);

    ~Node() OVERRIDE;

    Node* child(int row) const;

    Node* parentItem();

    int childCount() const;

    int row() const;

    void append(Node* item);
    void remove(int row);

    bool setData(const QModelIndex& index, const QVariant& value, int role);
    QVariant data(const QModelIndex& index, int role) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    double angle() const OVERRIDE;
    double tickness() const OVERRIDE;
    int count() const OVERRIDE;
    ThParam getParam() const;

    Point pos() const;
    AbstractThermPrGi* item() const;
    bool loadFile(QDataStream& stream) const;
    void disable() OVERRIDE;
    void enable() OVERRIDE;

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    bool isChecked() const OVERRIDE;
    QModelIndex index(int column = 0) const OVERRIDE;

    ThParam getPar() const;

private:
    const bool container = false;
    const QIcon icon;
    const QString name;
    const Point pos_;

    ThParam par;

    AbstractThermPrGi* const item_;

    Node* parent_ = nullptr;
    mvector<std::shared_ptr<Node>> childs;
    bool checked_ = false;

    Model* const model; // static wrong from anotherr dll
    static inline const Qt::CheckState chState[]{
        Qt::Unchecked,       // index 0
        Qt::Unchecked,       // index 1
        Qt::Checked,         // index 2
        Qt::PartiallyChecked // index 3
    };
};

#undef OVERRIDE

} // namespace Thermal
