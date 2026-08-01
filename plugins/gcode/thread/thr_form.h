/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_form.h"
#include "gc_plugin.h"
#include "thr_file.h"

namespace Ui {
class ThreadForm;
}

class QCheckBox;

namespace Threading {

// using PosOrPath = std::variant<const QPointF, const QPolygonF>;
// using Key = std::tuple<int, double, bool, QString>;
// struct Val {
// mvector<PosOrPath> posOrPath;
// Paths draw;
// };
// using Preview = std::map<Key, Val>;

class Model;
class Header;

class Form final : public GCode::Form {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin);
    ~Form() override;

    void updateFiles();
    static bool canToShow();

private:
    Ui::ThreadForm* ui;
    class Model* model       = nullptr;
    class AbstractFile* file = nullptr;
    class Header* header;
    class QCheckBox* checkBox;
    GCode::SideOfMilling side = GCode::Inner;

    void initToolTable();

    void on_cbxFileCurrentIndexChanged();
    void on_doubleClicked(const QModelIndex& current);
    void on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_customContextMenuRequested(const QPoint& pos);
    void customContextMenuRequested(const QPoint& pos);

    void pickUpTool();
    void updateState();
    //////////
    void errorOccurred();

    QModelIndexList selectedIndexes() const;

    inline void zoomToSelected();

protected:
    // FormsUtil interface
    void computePaths() override;
    void updateName() override;
    // QWidget interface
    void showEvent(QShowEvent* event) override {
        updateFiles();
        event->accept();
    }
    void hideEvent(QHideEvent* event) override;

public:
    void editFile(GCode::File* file) override;
};

class Plugin final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "description.json")
    Q_INTERFACES(GCode::Plugin)

    // GCode::Plugin interface
    Form form{this};

public:
    QAction* addAction(QMenu* menu, QToolBar* toolbar) override {
        auto action = GCode::Plugin ::addAction(menu, toolbar);
        action->setData(true);
        return action;
    }
    QIcon icon() const override { return QIcon::fromTheme(u"thread-path"_s); }
    QKeySequence keySequence() const override { return {u"Ctrl+Shift+T"_s}; }
    QWidget* createForm() override { return &form; }
    QString gcName() const override { return u"Threading"_s; }
    bool canToShow() const override { return Form::canToShow(); }
    uint32_t type() const override { return THREAD; }
    AbstractFile* loadFile(QDataStream& stream) const override { return File::load<File>(stream); }
};

} // namespace Threading

#include "app.h"
