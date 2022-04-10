/*******************************************************************************
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
#include "gc_formsutil.h"
#include "gc_thvars.h"

class ThermalModel;
class AbstractThermPrGi;
class QCheckBox;
class QItemSelection;

namespace Ui {
class ThermalForm;
}

namespace Gerber {
class AbstractAperture;
}

class ThermalForm : public FormsUtil {
    Q_OBJECT

public:
    explicit ThermalForm(QWidget* parent = nullptr);
    ~ThermalForm() override;

    void updateFiles();
    static bool canToShow();

private slots:
    void on_leName_textChanged(const QString& arg1);

    void on_cbxFileCurrentIndexChanged(int index);
    void on_dsbxDepth_valueChanged(double arg1);

    void on_dsbxAreaMax_editingFinished();
    void on_dsbxAreaMin_editingFinished();

private:
    Ui::ThermalForm* ui;

    void createTPI(FileInterface* file);

    mvector<std::shared_ptr<AbstractThermPrGi>> m_sourcePreview;

    ThermalModel* model = nullptr;
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void setSelection(const QModelIndex& selected, const QModelIndex& deselected);

    ThParam par;
    double lastMax;
    double lastMin;

    QCheckBox* chbx;
    double m_depth;
    inline void redraw();
    Tool tool;

    // FormsUtil interface
protected:
    void createFile() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

#include "gc_odeplugininterface.h"
#include <QToolBar>

class GCPluginImpl : public QObject, public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "thermal.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    GCPluginImpl(QObject* parent = nullptr)
        : QObject(parent) { }

    QObject* getObject() override { return this; }
    int type() const override { return GCode::Thermal; }
    QJsonObject info() const override {
        return {
            { "Name", "Thermal" },
            { "Version", "1.0" },
            { "VendorAuthor", "X-Ray aka Bakiev Damir" },
            { "Info", "Thermal" },
        };
    }
    QAction* addAction(QMenu* menu, QToolBar* toolbar) override {
        auto action = toolbar->addAction(icon(), info()["Name"].toString(), [this] {
            if (ThermalForm::canToShow())
                emit setDockWidget(new ThermalForm);
        });
        action->setShortcut(QKeySequence("Ctrl+Shift+T"));
        menu->addAction(action);
        return action;
    }

    QIcon icon() const override { return QIcon::fromTheme("thermal-path"); }

signals:
    void setDockWidget(QWidget*) override;
};
