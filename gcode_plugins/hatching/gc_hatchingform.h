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
    explicit HatchingForm(QWidget* parent = nullptr);
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

class GCPluginImpl : public QObject, public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "hatching.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    GCPluginImpl(QObject* parent = nullptr)
        : QObject(parent) { }

    QObject* getObject() override { return this; }
    int type() const override { return GCode::Hatching; }
    QJsonObject info() const override {
        return {
            { "Name", "Crosshatch" },
            { "Version", "1.0" },
            { "VendorAuthor", "X-Ray aka Bakiev Damir" },
            { "Info", "Crosshatch" },
        };
    }
    QAction* addAction(QMenu* menu, QToolBar* toolbar) override {
        auto action = toolbar->addAction(icon(), info()["Name"].toString(), [this] {
            emit setDockWidget(new HatchingForm);
        });
        action->setShortcut(QKeySequence("Ctrl+Shift+C"));
        menu->addAction(action);
        return action;
    }

    QIcon icon() const override { return QIcon::fromTheme("crosshatch-path"); }

signals:
    void setDockWidget(QWidget*) override;
};
