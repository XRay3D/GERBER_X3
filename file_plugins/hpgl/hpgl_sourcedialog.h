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

#include "hpgl_types.h"

#include <QtWidgets>

namespace Hpgl {

enum {
    LineNum,
    LineType,
    LineData,
    ColumnCount
};

class SourceDialog : public QDialog {
    Q_OBJECT
public:
    explicit SourceDialog(int fileId, QWidget* parent = nullptr);

signals:
};

//class Model : public QAbstractTableModel {
//    const mvector<QString>& lines;

//public:
//    Model(const mvector<QString>& lines, QObject* parent = nullptr)
//        : QAbstractTableModel(parent)
//        , lines(lines)
//    {
//    }
//    ~Model() { }

//    int rowCount(const QModelIndex& = {}) const override { return static_cast<int>(lines.size()); }
//    int columnCount(const QModelIndex& = {}) const override { return 3; }
//    Qt::ItemFlags flags(const QModelIndex& /*index*/) const override { return Qt::ItemIsEnabled | Qt::ItemIsSelectable; }

//    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
//    {
//        if (role == Qt::DisplayRole)
//            switch (index.column()) {
//            case LineNum:
//                return index.row() + 1;
//            case LineType:
//                return index.row() % 2 ? DxfObj::tr("Data") : DxfObj::tr("Code");
//            case LineData:
//                return lines.at(index.row());
//            }
//        else if (role == Qt::TextAlignmentRole)
//            switch (index.column()) {
//            case LineNum:
//                return Qt::AlignCenter;
//            case LineType:
//                return Qt::AlignCenter;
//            case LineData:
//                return Qt::AlignVCenter;
//            }
//        return {};
//    }

//    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
//    {
//        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
//            switch (section) {
//            case LineNum:
//                return DxfObj::tr("Line");
//            case LineType:
//                return DxfObj::tr("Type");
//            case LineData:
//                return DxfObj::tr("Data");
//            }
//        return {};
//    }
//};
}
