/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_formsutil.h"
#include "gc_plugin.h"

#include <QHeaderView>
#include <QMenu>
#include <QToolBar>
#include <QWidget>

namespace Ui {
class DrillForm;
}

class QCheckBox;

namespace DrillPlugin {

using PosOrPath = std::variant<const QPointF, const QPolygonF>;
using Key = std::tuple<int, double, bool, QString>;
struct Val {
    mvector<PosOrPath> posOrPath;
    Paths draw;
};
using Preview = std::map<Key, Val>;

class Model;
class Header;

class Form final : public GcFormBase {
    Q_OBJECT

public:
    explicit Form(GCodePlugin* plugin, QWidget* parent = nullptr);
    ~Form() override;

    void updateFiles();
    static bool canToShow();

private:
    Ui::DrillForm* ui;
    Model* model = nullptr;
    FileInterface* file = nullptr;
    Header* header;
    QCheckBox* checkBox;
    GCodePlugin* plugin;
    GCode::GCodeType worckType = GCode::Drill;
    GCode::SideOfMilling side = GCode::Inner;
    QString type_;

    void initToolTable();

    void on_cbxFileCurrentIndexChanged(int index);
    void on_doubleClicked(const QModelIndex& current);
    void on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_customContextMenuRequested(const QPoint& pos);
    void customContextMenuRequested(const QPoint& pos);

    void pickUpTool();
    void updateState();
    //////////
    void errorOccurred();

    QModelIndexList selectedIndexes() const;

    inline void zoomToSelected();

    // FormsUtil interface
protected:
    void createFile() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

class Plugin final : public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "drill.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("drill-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+D"}; }
    QWidget* createForm() override { return new Form(this); };
    bool canToShow() const override { return Form::canToShow(); }
    int type() const override { return GCode::Drill; }
};

} // namespace DrillPlugin

#include "app.h"
