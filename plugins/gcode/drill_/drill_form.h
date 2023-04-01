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

#include "file.h"
#include "gc_baseform.h"
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

class Form final : public GCode::BaseForm {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~Form() override;

    void updateFiles();
    static bool canToShow();

private:
    Ui::DrillForm* ui;
    Model* model = nullptr;
    AbstractFile* file = nullptr;
    Header* header;
    QCheckBox* checkBox;
    GCode::Plugin* plugin;
    GCodeType worckType = GCode::Drill;
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
    void сomputePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

class Plugin final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "drill.json")
    Q_INTERFACES(GCode::Plugin)

    // GCode::Plugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("drill-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+D"}; }
    QWidget* createForm() override { return new Form(this); };
    bool canToShow() const override { return Form::canToShow(); }
    uint32_t type() const override { return GCode::Drill; }
    AbstractFile* loadFile(QDataStream& stream) const override { return new GCode::DrillFile; }
};

} // namespace DrillPlugin

#include "app.h"
