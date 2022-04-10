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
#include "gc_odeplugininterface.h"

namespace Ui {
class ProfileForm;
}
class BridgeItem;

class ProfileForm : public FormsUtil {
    Q_OBJECT

public:
    explicit ProfileForm(QWidget* parent = nullptr);
    ~ProfileForm() override;

private slots:
    void on_pbAddBridge_clicked();
    void on_leName_textChanged(const QString& arg1);

private:
    Ui::ProfileForm* ui;
    double m_size = 0.0;
    double m_lenght = 0.0;
    void updateBridge();
    void updatePixmap();
    const QStringList names;
    static inline const QString pixmaps[] {
        QStringLiteral("prof_on_climb"),
        QStringLiteral("prof_out_climb"),
        QStringLiteral("prof_in_climb"),
        QStringLiteral("prof_on_conv"),
        QStringLiteral("prof_out_conv"),
        QStringLiteral("prof_in_conv"),
    };
    void rb_clicked();
    BridgeItem* brItem = nullptr;

    enum Trimming {
        Line = 1,
        Corner = 2,
    };
    int m_trimming = 0;

protected:
    // QWidget interface
    void
    resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    // FormsUtil interface
    void createFile() override;
    void updateName() override;

public:
    virtual void editFile(GCode::File* file) override;
};

#include <QToolBar>

class GCPluginImpl : public QObject, public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "profile.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    GCPluginImpl(QObject* parent = nullptr)
        : QObject(parent) { }

    QObject* getObject() override { return this; }
    int type() const override { return GCode::Profile; }
    QJsonObject info() const override {
        return {
            { "Name", "Pro&file" },
            { "Version", "1.0" },
            { "VendorAuthor", "X-Ray aka Bakiev Damir" },
            { "Info", "Profile" },
        };
    }
    QAction* addAction(QMenu* menu, QToolBar* toolbar) override {
        auto action = toolbar->addAction(icon(), info()["Name"].toString(), [this] {
            emit setDockWidget(new ProfileForm);
        });
        action->setShortcut(QKeySequence("Ctrl+Shift+F"));
        menu->addAction(action);
        return action;
    }

    QIcon icon() const override { return QIcon::fromTheme("profile-path"); }

signals:
    void setDockWidget(QWidget*) override;
};
