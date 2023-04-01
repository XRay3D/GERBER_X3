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

#include "gc_formsutil.h"
#include "gc_plugin.h"
#include <array>

namespace Ui {
class ProfileForm;
}
class GiBridge;

namespace Profile {

class ProfileForm : public GCode::FormBase {
    Q_OBJECT

public:
    explicit ProfileForm(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~ProfileForm() override;

private slots:
    void onAddBridgeClicked();
    void onNameTextChanged(const QString& arg1);

private:
    void updateBridges();
    void updatePixmap();
    void rb_clicked();

    Ui::ProfileForm* ui;
    //    GiBridge* brItem = nullptr;

    const QStringList names {tr("Profile On"), tr("Profile Outside"), tr("Profile Inside")};
    static inline const std::array pixmaps {
        QStringLiteral("prof_on_climb"),
        QStringLiteral("prof_out_climb"),
        QStringLiteral("prof_in_climb"),
        QStringLiteral("prof_on_conv"),
        QStringLiteral("prof_out_conv"),
        QStringLiteral("prof_in_conv"),
    };

    enum Trimming {
        Line = 1,
        Corner = 2,
    };

    enum BridgeAlign {
        Manually,
        Horizontally,
        Vertically,
        HorizontallyVertically,
        ThroughTheDistance,
        EvenlyDround,
    };

    void updateBridgePos(QPointF pos);

    int trimming_ = 0;

protected:
    // QWidget interface
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    // FormsUtil interface
    void сomputePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

#include "profile.h"
#include <QToolBar>

class GCPluginImpl final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "profile.json")
    Q_INTERFACES(GCode::Plugin)

    // GCode::Plugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("profile-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+F"}; }
    QWidget* createForm() override { return new ProfileForm(this); };
    uint32_t type() const override { return md5::hash32("Profile"); }
    AbstractFile* loadFile(QDataStream& stream) const override { return new GCode::File; }
};

} // namespace Profile
