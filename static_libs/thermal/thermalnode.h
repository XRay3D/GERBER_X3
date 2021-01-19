/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "thermalpreviewitem.h"

#include "thvars.h"

#include <QIcon>
#include <QModelIndex>
#include <memory>

class ThermalModel;
class ThermalNode;

class ThermalNodeI {
public:
    virtual ~ThermalNodeI() { }
    virtual bool isChecked() const = 0;
    virtual void disable() = 0;
    virtual void enable() = 0;
    virtual QModelIndex index(int column = 0) const = 0;

    virtual double angle() const = 0;
    virtual double tickness() const = 0;
    virtual int count() const = 0;
};

class __declspec(dllexport) ThermalNode final : public ThermalNodeI {
public:
    explicit ThermalNode(const QIcon& icon, const QString& name, const ThParam& par, const IntPoint& pos, AbstractThermPrGi* item, ThermalModel* model);
    explicit ThermalNode(const QIcon& icon, const QString& name, const ThParam& par, ThermalModel* model);
    explicit ThermalNode(ThermalModel* model);

    ~ThermalNode() override;

    ThermalNode* child(int row) const;

    ThermalNode* parentItem();

    int childCount() const;

    int row() const;

    void append(ThermalNode* item);
    void remove(int row);

    bool setData(const QModelIndex& index, const QVariant& value, int role);
    QVariant data(const QModelIndex& index, int role) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    double angle() const override;
    double tickness() const override;
    int count() const override;
    ThParam getParam() const;

    IntPoint pos() const;
    AbstractThermPrGi* item() const;
    bool createFile() const;
    void disable() override;
    void enable() override;

    ThermalNode(const ThermalNode&) = delete;
    ThermalNode& operator=(const ThermalNode&) = delete;

    bool isChecked() const override;
    QModelIndex index(int column = 0) const override;

    ThParam getPar() const;

private:
    const bool container = false;
    const QIcon icon;
    const QString name;
    const IntPoint m_pos;

    ThParam par;

    AbstractThermPrGi* const m_item;

    ThermalNode* m_parent = nullptr;
    mvector<std::shared_ptr<ThermalNode>> childs;
    bool m_checked = false;

    ThermalModel* const model; // static wrong from anotherr dll
    static inline const Qt::CheckState chState[] {
        Qt::Unchecked, // index 0
        Qt::Unchecked, // index 1
        Qt::Checked, // index 2
        Qt::PartiallyChecked // index 3
    };
};
