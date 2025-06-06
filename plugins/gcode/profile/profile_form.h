/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_baseform.h"
#include "gc_plugin.h"
#include "profile.h"
#include <QToolBar>
#include <array>

namespace Ui {
class ProfileForm;
}

namespace Gi {
class Bridge;
}

namespace Profile {

class Form : public GCode::BaseForm {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~Form() override;

private slots:
    void onAddBridgeClicked();
    void onNameTextChanged(const QString& arg1);

private:
    void updateBridges();
    void updatePixmap();
    void rb_clicked();

    Ui::ProfileForm* ui;
    //    Gi::Bridge* brItem = nullptr;

    const QStringList names{tr("Profile On"), tr("Profile Outside"), tr("Profile Inside")};
    static inline const std::array pixmaps{
        u"prof_on_climb"_s,
        u"prof_out_climb"_s,
        u"prof_in_climb"_s,
        u"prof_on_conv"_s,
        u"prof_out_conv"_s,
        u"prof_in_conv"_s,
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
        Split,
    };

    void updateBridgePos(QPointF pos);

    int trimming_ = 0;

protected:
    // QWidget interface
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    // FormsUtil interface
    void computePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

class GCPluginImpl final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "description.json")
    Q_INTERFACES(GCode::Plugin)
    Form form{this};

public:
    // GCode::Plugin interface
    QIcon icon() const override { return QIcon::fromTheme("profile-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+F"}; }
    QWidget* createForm() override { return &form; }
    uint32_t type() const override { return md5::hash32("Profile"); }
    AbstractFile* /*GCode::File*/ loadFile(QDataStream& stream) const override { return File::load<File>(stream); }

    // AbstractFilePlugin interface
    AbstractFileSettings* createSettingsTab(QWidget* parent) override {
        class Tab : public AbstractFileSettings {
            QComboBox* cbxProfileSort;

        public:
            Tab(QWidget* parent)
                : AbstractFileSettings{parent} {
                setWindowTitle(tr("Profile"));
                auto lbl = new QLabel{QApplication::translate("Profile", "Milling sequence:", nullptr), this};

                cbxProfileSort = new QComboBox{this};
                cbxProfileSort->setObjectName(u"cbxProfileSort"_s);
                cbxProfileSort->addItem(QApplication::translate("Profile", "Grouping by nesting"));
                cbxProfileSort->addItem(QApplication::translate("Profile", "Grouping by nesting depth"));

                auto layout = new QVBoxLayout{this};
                layout->setContentsMargins(6, 6, 6, 6);
                layout->addWidget(lbl);
                layout->addWidget(cbxProfileSort);
                layout->addStretch();
            }
            ~Tab() override = default;
            void readSettings(MySettings& settings) override {
                Profile::settings.sort = settings.getValue(cbxProfileSort, Profile::settings.sort);
            }
            void writeSettings(MySettings& settings) override {
                Profile::settings.sort = settings.setValue(cbxProfileSort);
            }
        };
        return new Tab{parent};
    }
};

} // namespace Profile
