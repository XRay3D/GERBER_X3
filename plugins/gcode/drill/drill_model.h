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

#include "mvector.h"
#include <QAbstractTableModel>
#include <QIcon>

namespace Drilling {

class GiPreview;

struct Row {
    //    Row(QString&& name = {},
    //        QIcon&& icon = {},
    //        int32_t id = {},
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
    mvector<GiPreview*> items;
};

class Model : public QAbstractTableModel {
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
    explicit Model(QString type, int rowCount, QObject* parent = nullptr);
    ~Model() override { qDebug(__FUNCTION__); }

    bool isSlot(int row) const { return data_[row].isSlot; }
    bool useForCalc(int row) const { return data_[row].useForCalc; }

    int apertureId(int row) const { return data_[row].apertureId; }
    int toolId(int row) const { return data_[row].toolId; }

    void setCreate(bool create);
    void setCreate(int row, bool create);
    void setToolId(int row, int32_t id);
    //    void setType(int type_) { type = type_; }

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    mvector<Row>& data() { return data_; }
    const mvector<Row>& data() const { return data_; }
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
};

} // namespace Drilling
