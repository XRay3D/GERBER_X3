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
#include <QAbstractTableModel>
#include <QIcon>

enum {
    tAperture,
    tTool,
};

struct Row {
    Row(const QString& name = {}, const QIcon& icon = {}, int id = {})
        : name(name)
        , icon(icon)
        , apertureId(id)
        , toolId(-1)
    {
    }
    const QString name;
    const QIcon icon;
    const int apertureId;
    int toolId;
    bool isSlot = false;
    bool useForCalc = false;
    inline static double depth;
};

class DrillModel : public QAbstractTableModel {
    Q_OBJECT

    QVector<Row> m_data;
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
    Row& appendRow(const QString& name, const QIcon& icon, int id);
    void setToolId(int row, int id);
    int toolId(int row);
    void setSlot(int row, bool slot);
    bool isSlot(int row);
    int apertureId(int row);
    bool useForCalc(int row) const;
    void setCreate(int row, bool create);
    void setCreate(bool create);

    const QVector<Row>& data() const { return m_data; }

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};
