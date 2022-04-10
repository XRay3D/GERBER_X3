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
class VoronoiForm;
}

class VoronoiForm : public FormsUtil {
    Q_OBJECT

public:
    explicit VoronoiForm(QWidget* parent = nullptr);
    ~VoronoiForm() override;

private slots:
    void on_leName_textChanged(const QString& arg1);
    void on_cbxSolver_currentIndexChanged(int index);

private:
    Ui::VoronoiForm* ui;

    double m_size = 0.0;
    double m_lenght = 0.0;
    void setWidth(double w);

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
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "voronoi.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    GCPluginImpl(QObject* parent = nullptr)
        : QObject(parent) { }

    QObject* getObject() override { return this; }
    int type() const override { return GCode::Voronoi; }
    QJsonObject info() const override {
        return {
            { "Name", "Voronoi" },
            { "Version", "1.0" },
            { "VendorAuthor", "X-Ray aka Bakiev Damir" },
            { "Info", "Voronoi" },
        };
    }
    QAction* addAction(QMenu* menu, QToolBar* toolbar) override {
        auto action = toolbar->addAction(icon(), info()["Name"].toString(), [this] {
            emit setDockWidget(new VoronoiForm);
        });
        action->setShortcut(QKeySequence("Ctrl+Shift+V"));
        menu->addAction(action);
        return action;
    }

    QIcon icon() const override { return QIcon::fromTheme("voronoi-path"); }

signals:
    void setDockWidget(QWidget*) override;
};
