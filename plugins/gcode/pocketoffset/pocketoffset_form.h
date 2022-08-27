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
class PocketOffsetForm;
}

class PocketOffsetForm : public FormsUtil {
    Q_OBJECT

public:
    explicit PocketOffsetForm(GCodePlugin* plugin, QWidget* parent = nullptr);
    ~PocketOffsetForm() override;

private slots:
    void on_sbxSteps_valueChanged(int arg1);
    void on_leName_textChanged(const QString& arg1);

private:
    Ui::PocketOffsetForm* ui;

    int direction = 0;
    void updatePixmap();
    void rb_clicked();
    const QStringList names;
    static inline const QString pixmaps[] {
        QStringLiteral("pock_offs_climb"),
        QStringLiteral("pock_offs_conv"),
    };
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

#include "gc_plugin.h"
#include <QToolBar>

class GCPluginImpl final : public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "pocketoffset.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("pocket-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+P"}; }
    QWidget* createForm() override { return new PocketOffsetForm(this); };
    int type() const override { return GCode::Pocket; }
};
