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
    explicit PocketOffsetForm(QWidget* parent = nullptr);
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

#include "gc_odeplugininterface.h"
#include <QToolBar>

class GCPluginImpl : public QObject, public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "pocketoffset.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    GCPluginImpl(QObject* parent = nullptr)
        : QObject(parent) { }

    QObject* getObject() override { return this; }
    int type() const override { return GCode::Pocket; }
    QJsonObject info() const override {
        return {
            { "Name", "Pocket Offset" },
            { "Version", "1.0" },
            { "VendorAuthor", "X-Ray aka Bakiev Damir" },
            { "Info", "Pocket Offset" },
        };
    }
    QAction* addAction(QMenu* menu, QToolBar* toolbar) override {
        auto action = toolbar->addAction(icon(), info()["Name"].toString(), [this] {
            emit setDockWidget(new PocketOffsetForm);
        });
        action->setShortcut(QKeySequence("Ctrl+Shift+P"));
        menu->addAction(action);
        return action;
    }

    QIcon icon() const override { return QIcon::fromTheme("pocket-path"); }

signals:
    void setDockWidget(QWidget*) override;
};
