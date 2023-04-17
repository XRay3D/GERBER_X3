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

#include "gc_baseform.h"
#include "gc_plugin.h"
#include "thread.h"
#include <QToolBar>
#include <array>

namespace Ui {
class ThreadForm;
}
class GiBridge;

namespace Thread {

class Form : public GCode::BaseForm {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~Form() override;

private slots:
    void onNameTextChanged(const QString& arg1);

private:
    void updatePixmap();
    void rb_clicked();

    Ui::ThreadForm* ui;
    //    GiBridge* brItem = nullptr;

    const QStringList names{tr("Thread On"), tr("Thread Outside"), tr("Thread Inside")};
    static inline const std::array pixmaps{
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
    void computePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

class GCPluginImpl final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "thread.json")
    Q_INTERFACES(GCode::Plugin)
 Form form{this};
public:
    // GCode::Plugin interface
    QIcon icon() const override { return QIcon::fromTheme("crosshairs"); } // FIXME
    QKeySequence keySequence() const override { return {"Ctrl+Shift+F"}; }
    QWidget* createForm() override{return &form;};
    uint32_t type() const override { return md5::hash32("Thread"); }
    AbstractFile* /*GCode::File*/ loadFile(QDataStream& stream) const override { return File::load<File>(stream); }

    // AbstractFilePlugin interface
    AbstractFileSettings* createSettingsTab(QWidget* parent) override {
        class Tab : public AbstractFileSettings {
            QComboBox* cbxThreadSort;

        public:
            Tab(QWidget* parent)
                : AbstractFileSettings{parent} {
                setWindowTitle(tr("Thread"));
                auto lbl = new QLabel(QApplication::translate("Thread", "Milling sequence:", nullptr), this);

                cbxThreadSort = new QComboBox(this);
                cbxThreadSort->setObjectName(QString::fromUtf8("cbxThreadSort"));
                cbxThreadSort->addItem(QApplication::translate("Thread", "Grouping by nesting"));
                cbxThreadSort->addItem(QApplication::translate("Thread", "Grouping by nesting depth"));

                auto layout = new QVBoxLayout(this);
                layout->setContentsMargins(6, 6, 6, 6);
                layout->addWidget(lbl);
                layout->addWidget(cbxThreadSort);
                layout->addStretch();
            }
            ~Tab() override = default;
            void readSettings(MySettings& settings) override {
                Thread::settings.sort = settings.getValue(cbxThreadSort, Thread::settings.sort);
            }
            void writeSettings(MySettings& settings) override {
                Thread::settings.sort = settings.setValue(cbxThreadSort);
            }
        };
        return new Tab(parent);
    }
};

} // namespace Thread
