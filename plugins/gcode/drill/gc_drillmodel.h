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

enum {
    tAperture,
    tTool,
};

struct Row {
    Row(QString&& name = {},
        QIcon&& icon = {},
        int id = {},
        double diameter = {})
        : icon(icon)
        , name(name)
        , diameter(diameter)
        , apertureId(id)
        , toolId(-1) {
    }
    const QIcon icon;
    const QString name;
    const double diameter;
    const int apertureId;
    bool isSlot = false;
    bool useForCalc = false;
    int toolId;
};

class DrillModel : public QAbstractTableModel {
    Q_OBJECT

    mvector<Row> m_data;
    int m_type;

    enum {
        Name,
        Tool,
        ColumnCount
    };

signals:
    void set(int, bool);

public:
    DrillModel(int type, int rowCount, QObject* parent = nullptr);
    DrillModel(QObject* parent = nullptr);

    //    Row& appendRow(const QString& name, const QIcon& icon, int id);
    void setToolId(int row, int id);
    int toolId(int row);
    void setSlot(int row, bool slot);
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
    void setType(int type) { m_type = type; }

    mvector<Row>& data() { return m_data; }
    const mvector<Row>& data() const { return m_data; }
};
