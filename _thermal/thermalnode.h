/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#ifdef GERBER
#include "thermalpreviewitem.h"
#include "thvars.h"
#include <QIcon>
#include <QModelIndex>

class ThermalModel;

class ThermalNode {
public:
    ThermalNode(const QIcon& icon, const QString& name, const ThParam& par, const Point64& pos, ThermalPreviewItem* item);
    ThermalNode(const QIcon& icon, const QString& name, const ThParam& par);
    explicit ThermalNode(ThermalModel* _model);

    ~ThermalNode();

    ThermalNode* child(int row) const;

    ThermalNode* parentItem();

    int childCount() const;

    int row() const;

    void append(ThermalNode* item);
    void remove(int row);

    bool setData(const QModelIndex& index, const QVariant& value, int role);
    QVariant data(const QModelIndex& index, int role) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    double angle() const;
    double tickness() const;
    int count() const;
    ThParam getParam() const { return par; };

    Point64 pos() const;
    ThermalPreviewItem* item() const;
    bool createFile() const;
    void disable();
    void enable();

    ThermalNode(const ThermalNode&) = delete;
    ThermalNode& operator=(const ThermalNode&) = delete;

    bool isChecked() const;
    QModelIndex index(int column = 0) const;

private:
    const bool container = false;
    const QIcon icon;
    const QString name;
    const Point64 m_pos;

    ThParam par;

    ThermalPreviewItem* const m_item = nullptr;

    ThermalNode* m_parent = nullptr;
    QList<QSharedPointer<ThermalNode>> childs;
    bool m_checked = false;

    inline static ThermalModel* model;
    inline static const Qt::CheckState chState[] {
        Qt::Unchecked, // index 0
        Qt::Unchecked, // index 1
        Qt::Checked, // index 2
        Qt::PartiallyChecked // index 3
    };
};
#endif
