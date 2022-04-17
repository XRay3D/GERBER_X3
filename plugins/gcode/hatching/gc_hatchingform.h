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

namespace Ui {
class HatchingForm;
}

class HatchingForm : public FormsUtil {
    Q_OBJECT

public:
    explicit HatchingForm(GCodePlugin* plugin, QWidget* parent = nullptr);
    ~HatchingForm();

private slots:
    void on_leName_textChanged(const QString& arg1);

private:
    Ui::HatchingForm* ui;

    int direction = 0;
    void updatePixmap();
    void rb_clicked();
    const QStringList names;
    const QStringList pixmaps;
    // QWidget interface
protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

    // FormsUtil interface
protected:
    void createFile() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

#include "gc_odeplugininterface.h"
#include <QToolBar>

class GCPluginImpl : public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "hatching.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("crosshatch-path"); }
    QKeySequence keySequence() const override { return { "Ctrl+Shift+C" }; }
    QWidget* createForm()  override { return new HatchingForm(this); };
    int type() const override { return GCode::Hatching; }
};
