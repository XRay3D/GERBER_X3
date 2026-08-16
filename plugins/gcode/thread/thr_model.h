/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "tool.h"
#include <QAbstractTableModel>
#include <QIcon>

namespace Threading {

namespace Gi {
class Preview;
}

struct Row {
    // Row(QString&& name = {},
    // QIcon&& icon = {},
    // int32_t id = {},
    // double diameter = {})
    // : icon{icon}
    // , name(name)
    // , diameter(diameter)
    // , rowId(id)
    // , toolId(-1) {
    // }
    ~Row();
    /*const*/ QIcon icon;
    /*const*/ QStringList name;
    /*const*/ double diameter;
    bool useForCalc{};
    Tool::ID toolId{};
    double correction{};
    double holeDiameter{};
    std::vector<Gi::Preview*> items;
};

class Model : public QAbstractTableModel {
    Q_OBJECT

    std::vector<Row> data_;

public:
    enum {
        Name,
        Tool,
        Correction,
        HoleDiameter,
        ColumnCount
    };

signals:
    void set(int, bool);

public:
    explicit Model(size_t rowCount, QObject* parent = nullptr);
    ~Model() override { qDebug(__FUNCTION__); }

    bool useForCalc(int row) const { return data_[row].useForCalc; }

    Tool::ID toolId(int row) const { return data_[row].toolId; }
    double correction(int row) const { return data_[row].correction; }
    double holeDiameter(int row) const { return data_[row].holeDiameter; }

    void setCreate(bool create);
    void setCreate(int row, bool create);
    void setToolId(int row, Tool::ID id);
    void setCorrection(int row, double value);
    void setHoleDiameter(int row, double value);
    // void setType(int type_) { type = type_; }

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    std::vector<Row>& data() { return data_; }
    const std::vector<Row>& data() const { return data_; }
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
};

} // namespace Threading
