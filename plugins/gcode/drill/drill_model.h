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
#pragma once

#include "mvector.h"
#include <QAbstractTableModel>
#include <QIcon>

class GiDrillPreview;

struct Row {
    //    Row(QString&& name = {},
    //        QIcon&& icon = {},
    //        int id = {},
    //        double diameter = {})
    //        : icon(icon)
    //        , name(name)
    //        , diameter(diameter)
    //        , apertureId(id)
    //        , toolId(-1) {
    //    }
    ~Row() { qDeleteAll(items); }
    const QIcon icon;
    const QString name;
    const double diameter;
    const int apertureId;
    const bool isSlot;
    bool useForCalc {};
    int toolId {-1};
    mvector<GiDrillPreview*> items;
};

class DrillModel : public QAbstractTableModel {
    Q_OBJECT

    mvector<Row> data_;
    QString type;

    enum {
        Name,
        Tool,
        ColumnCount
    };

signals:
    void set(int, bool);

public:
    DrillModel(QString type, int rowCount, QObject* parent = nullptr);
    DrillModel(QObject* parent = nullptr);

    void setToolId(int row, int id);
    int toolId(int row);
    bool isSlot(int row);
    int apertureId(int row);
    bool useForCalc(int row) const;
    void setCreate(int row, bool create);
    void setCreate(bool create);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void setType(int type) { type = type; }

    mvector<Row>& data() { return data_; }
    const mvector<Row>& data() const { return data_; }
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
};
