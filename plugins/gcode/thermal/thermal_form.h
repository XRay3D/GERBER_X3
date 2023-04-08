/*******************************************************************************
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
#include <QToolBar>

#include "gc_baseform.h"
#include "gc_plugin.h"
#include "thermal.h"
#include "thermal_vars.h"
#include <QToolBar>

class Model;
class QCheckBox;
class QItemSelection;

namespace Ui {
class ThermalForm;
}

namespace Thermal {

class AbstractThermPrGi;

class Form : public GCode::BaseForm {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~Form() override;
    void updateFiles();

private:
    // slots
    void onDsbxAreaMinEditingFinished();
    void onDsbxAreaMaxEditingFinished();
    void onDsbxDepthValueChanged(double arg1);
    void onNameTextChanged(const QString& arg1);

    Ui::ThermalForm* ui;

    void updateThermalGi();

    mvector<std::shared_ptr<AbstractThermPrGi>> items_;
    PreviewGiMap thPaths;

    Model* model = nullptr;
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void setSelection(const QModelIndex& selected, const QModelIndex& deselected);

    void updateCriterias();
    std::vector<Criteria> criterias;

    ThParam par;
    double lastMax;
    double lastMin;

    QCheckBox* chbx;
    double depth_;
    inline void redraw();
    Tool tool;

    // FormsUtil interface
protected:
    void computePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

class Plugin final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "thermal.json")
    Q_INTERFACES(GCode::Plugin)

    // GCode::Plugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("thermal-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+T"}; }
    QWidget* createForm() override { return new Form(this); };
    //    bool canToShow() const override { return /*Form::canToShow()*/; }
    uint32_t type() const override { return THERMAL; }
    AbstractFile* loadFile(QDataStream& stream) const override { return File::load<File>(stream); }
};

} // namespace Thermal
