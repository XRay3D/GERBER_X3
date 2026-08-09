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

#include "drill_file.h"
#include "gc_form.h"
#include "gc_plugin.h"

namespace Ui {
class DrillForm;
}

class QCheckBox;

namespace Drilling {

// using PosOrPath = std::variant<const QPointF, const QPolygonF>;
// using Key = std::tuple<int, double, bool, QString>;
// struct Val {
// std::vector<PosOrPath> posOrPath;
// Paths64 draw;
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
    Ui::DrillForm* ui;
    class Model* model = nullptr;
    class AbstractFile* file = nullptr;
    class Header* header;
    class QCheckBox* checkBox;
    GCType worckType = GCType::Drill;
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
    // Файлы, созданные чужим Creator'ом (Profile/PocketOffset), тоже должны
    // унести с собой строки таблицы -- поэтому обработчик расширяется.
    void fileHandler(GCode::File* file) override;

    // Строки таблицы с данными индексами -- в том виде, в каком их надо будет
    // восстановить при правке.
    std::vector<RowRef> rowRefs(const std::vector<int>& indexes) const;
    // Строки текущего прогона: их нечем передать через Creator, поэтому лежат
    // здесь между emit createToolpath и приходом файла.
    std::vector<RowRef> pendingRows_;
    // QWidget interface
    void showEvent(QShowEvent* event) override {
        updateFiles();
        // Базовая, а не event->accept(): в GCode::Form::showEvent сбрасывается
        // режим правки, иначе форма молча перепишет УП из прошлого сеанса.
        GCode::Form::showEvent(event);
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
    QIcon icon() const override { return QIcon::fromTheme(u"drill-path"_s); }
    QKeySequence keySequence() const override { return {u"Ctrl+Shift+D"_s}; }
    QWidget* createForm() override { return &form; }
    QString gcName() const override { return u"Drilling"_s; }
    bool canToShow() const override { return Form::canToShow(); }
    uint32_t type() const override { return DRILLING; }
    AbstractFile* loadFile(QDataStream& stream) const override { return File::load<File>(stream); }
};

} // namespace Drilling

#include "app.h"
