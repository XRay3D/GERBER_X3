/*******************************************************************************
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
#include <QToolBar>

#include "gc_form.h"
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

class Form : public GCode::Form {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin);
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
    // QWidget interface
    void showEvent(QShowEvent* event) override {
        updateFiles();
        // Базовая, а не event->accept(): в GCode::Form::showEvent сбрасывается
        // режим правки, иначе форма молча перепишет УП из прошлого сеанса.
        GCode::Form::showEvent(event);
    }

    void hideEvent(QHideEvent* event) override;

public:
    void editFile(GCode::File* file) override;
};

class Plugin final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "description.json")
    Q_INTERFACES(GCode::Plugin)
    Form form{this};
    // GCode::Plugin interface
public:
    QAction* addAction(QMenu* menu, QToolBar* toolbar) override {
        auto action = GCode::Plugin ::addAction(menu, toolbar);
        action->setData(true);
        return action;
    }
    QIcon icon() const override { return QIcon::fromTheme(u"thermal-path"_s); }
    QKeySequence keySequence() const override { return {u"Ctrl+Shift+T"_s}; }
    QWidget* createForm() override { return &form; }
    QString gcName() const override { return u"Thermal"_s; };
    // bool canToShow() const override { return /*Form::canToShow()*/; }
    uint32_t type() const override { return THERMAL; }
    AbstractFile* loadFile(QDataStream& stream) const override { return File::load<File>(stream); }
};

} // namespace Thermal
