/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "topor_component.h"
#include <QAbstractTableModel>

namespace TopoR {

// Компонентный браузер -- список позиционных обозначений/BOM-таблица, как у
// Gerber (gbrcomp_model.h), но по нативным полям TopoR (см. topor_component.h).
class ComponentModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit ComponentModel(const Components& components, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    enum Column {
        RefDes,
        Footprint,
        ComponentRef,
        Side,
        Angle,
        X,
        Y,
        ColumnCount
    };

private:
    const Components& components_;
};

} // namespace TopoR
