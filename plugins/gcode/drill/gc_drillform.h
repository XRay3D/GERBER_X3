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

#include "gc_plugin.h"
#include "gcode.h"

#include <QHeaderView>
#include <QMenu>
#include <QToolBar>
#include <QWidget>

using HV = std::variant<const QPointF, const QPolygonF>;
using HK = std::tuple<int, double, bool, QString>;
using HoleMap = std::map<HK, mvector<HV>>;

namespace Ui {
class DrillForm;
}

class GiAbstractPreview;
class DrillModel;
class Header;
class QCheckBox;

class DrillForm : public QWidget {
    Q_OBJECT

public:
    explicit DrillForm(GCodePlugin* plugin, QWidget* parent = nullptr);
    ~DrillForm() override;

    void updateFiles();
    static bool canToShow();

public slots:
    void on_pbClose_clicked();

private slots:
    void on_pbCreate_clicked();

private:
    Ui::DrillForm* ui;
    DrillModel* model = nullptr;
    FileInterface* file = nullptr;
    Header* header;
    QCheckBox* checkBox;
    GCodePlugin* plugin;
    GCode::GCodeType worckType = GCode::Drill;
    GCode::SideOfMilling side = GCode::Inner;
    QString type_;

    void on_cbxFileCurrentIndexChanged(int index);
    void on_doubleClicked(const QModelIndex& current);
    void on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_customContextMenuRequested(const QPoint& pos);

    void pickUpTool();
    QModelIndexList selectedIndexes() const;

    inline void zoomToSelected();
};

#include "app.h"
