/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "script_engine.h"
#include "shapepluginin.h"

#include <QAbstractTableModel>
#include <QComboBox>
#include <QLabel>
#include <QTableView>

namespace ShScript {

class Shape;

/// Строка 0 -- центр (X/Y), дальше -- по строке на параметр скрипта
/// первого выделенного шейпа (в порядке объявления в скрипте).
class Model : public QAbstractTableModel {
    Q_OBJECT
    friend class Shape;
    ScriptRegistry& registry_;
    std::vector<ParamDef> defs_; // параметры опорного (первого выделенного) шейпа

public:
    Model(ScriptRegistry& registry, QObject* parent);
    ~Model() override = default;

    enum Row : int { Center };

    /// Перечитать список параметров с выделенных шейпов; вызывать перед reset().
    void sync();
    const ParamDef* defAt(int row) const;

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override { return 1 + int(defs_.size()); }
    int columnCount(const QModelIndex& = {}) const override { return 2; }
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& /*index*/) const override { return Qt::ItemIsEditable | Qt::ItemIsEnabled; }
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    std::vector<Shape*> shapes;
};

class Editor : public Shapes::Editor {
    Q_OBJECT

    ScriptRegistry& registry_;
    QComboBox* scriptBox;
    QLabel* errorLabel;
    QTableView* view;
    bool resetFl{};

    void applySpans();
    void updateError();

public:
    Editor(Shapes::Plugin* plugin, ScriptRegistry& registry);
    ~Editor() override = default;

    /// Имя скрипта, выбранного в комбобоксе (для новых фигур).
    QString currentScript() const { return scriptBox->currentText(); }
    /// Перезаполнить комбобокс списком *.js из папки, сохранив выбор.
    void refreshScripts();
    /// Пересобрать таблицу под текущее выделение.
    void refresh();

    void add(Shapes::AbstractShape* shape) override;
    void remove(Shapes::AbstractShape* shape) override;
    void updateData() override;

    Model* model;
    Shapes::Plugin* plugin;
};

} // namespace ShScript
