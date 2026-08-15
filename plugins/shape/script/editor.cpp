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
#include "editor.h"
#include "doublespinbox.h"
#include "shape.h"

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <array>
#include <set>

Q_DECLARE_METATYPE(std::set<double>)

namespace ShScript {

static auto selected(const std::vector<Shape*>& shapes) {
    return shapes | v::filter([](Shape* sh) { return sh->isSelected(); });
}

//////////////////////////////////////////
/// \brief Model::Model
Model::Model(ScriptRegistry& registry, QObject* parent)
    : QAbstractTableModel{parent}
    , registry_{registry} { }

void Model::sync() {
    // Число строк меняется -- заголовкам нужен полноценный reset модели,
    // view->reset() один их не пересчитает.
    beginResetModel();
    defs_.clear();
    for(auto* shape: selected(shapes)) {
        if(auto* script = shape->scriptDef()) {
            // Только те параметры, что реально есть у шейпа (после merge -- все).
            for(const auto& def: script->params)
                if(shape->params().contains(def.name)) defs_.push_back(def);
        } else {
            for(const auto& [name, value]: shape->params()) defs_.push_back({.name = name, .description = {}, .value = value});
        }
        break;
    }
    endResetModel();
}

const ParamDef* Model::defAt(int row) const {
    return row > Center && size_t(row - 1) < defs_.size() ? &defs_[size_t(row - 1)] : nullptr;
}

QVariant Model::data(const QModelIndex& index, int role) const {
    auto sh = selected(shapes);
    static const std::array getter{&QPointF::x, &QPointF::y};
    if(r::empty(sh)) return {};

    auto set = [&] {
        std::set<double> set;
        if(index.row() == Center) {
            for(auto* shape: sh)
                set.emplace((shape->handles[Center].*getter[index.column()])());
        } else if(auto* def = defAt(index.row())) {
            for(auto* shape: sh)
                if(auto it = shape->params().find(def->name); it != shape->params().end())
                    set.emplace(it->second);
        }
        return set;
    };

    if(role == Qt::DisplayRole) {
        QString ret;
        for(auto val: set())
            ret += (ret.size() ? u" | " : u"") + QString::number(val);
        return ret;
    }

    if(role == Qt::EditRole)
        return QVariant::fromValue(set());

    if(role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;

    return {};
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole) {
        if(orientation == Qt::Vertical) {
            if(section == Center) return tr(" Center");
            if(auto* def = defAt(section)) return QString{u' ' + def->label()};
            return {};
        }
        return section ? u"Y"_s : u"X"_s;
    }
    if(role == Qt::ToolTipRole && orientation == Qt::Vertical)
        if(auto* def = defAt(section)) return def->name;
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    auto sh = selected(shapes);
    static const std::array setter{&Shapes::Handle::setX, &Shapes::Handle::setY};

    if(role != Qt::EditRole || r::empty(sh)) return {};

    const double val = value.toDouble();
    if(index.row() == Center) {
        for(auto* shape: sh) {
            (shape->handles[Center].*setter[index.column()])(val);
            shape->curHandle = shape->handles.data() + Center;
            shape->redraw();
        }
    } else if(auto* def = defAt(index.row())) {
        for(auto* shape: sh) shape->setParam(def->name, val);
    } else
        return {};
    return true;
}

//////////////////////////////////////////
/// Delegate
class Delegate : public QStyledItemDelegate {
    Model* model_;
    mutable QDoubleSpinBox* dsbx{};
    mutable double last{};

public:
    Delegate(Model* model, QObject* parent)
        : QStyledItemDelegate{parent}
        , model_{model} { }
    ~Delegate() override = default;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const override {
        dsbx = new DoubleSpinBox{parent};
        if(auto* def = model_->defAt(index.row())) {
            dsbx->setRange(def->min, def->max);
            dsbx->setSingleStep(def->step);
            dsbx->setDecimals(def->decimals);
        } else {
            dsbx->setRange(-10000, +10000);
            dsbx->setDecimals(3);
        }
        connect(dsbx, &DoubleSpinBox::valueChanged, this, &Delegate::emitCommitData);
        return dsbx;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        dsbx = static_cast<QDoubleSpinBox*>(editor);
        auto set = index.data(Qt::EditRole).value<std::set<double>>();
        if(set.size()) dsbx->setValue(last = *set.begin());
        last = dsbx->value();
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        dsbx = static_cast<QDoubleSpinBox*>(editor);
        if(qFuzzyCompare(last, dsbx->value())) return;
        last = dsbx->value();
        model->setData(index, dsbx->value());
    }

    void emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }
};

//////////////////////////////////////////
/// \brief Editor::Editor
Editor::Editor(Shapes::Plugin* plugin, ScriptRegistry& registry)
    : registry_{registry}
    , scriptBox{new QComboBox{this}}
    , errorLabel{new QLabel{this}}
    , view{new QTableView{this}}
    , model{new Model{registry, view}}
    , plugin{plugin} {
    setWindowTitle(plugin->name());

    auto vLayout = new QVBoxLayout{this};
    vLayout->setContentsMargins(6, 6, 6, 6);
    vLayout->setSpacing(6);
    {
        auto hLayout = new QHBoxLayout;
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(6);
        hLayout->addWidget(new QLabel{tr("Script:"), this});
        scriptBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        hLayout->addWidget(scriptBox, 1);

        auto toolButton = [&](const QString& icon, const QString& tip, auto&& slot) {
            auto* action = new QAction{QIcon::fromTheme(icon), tip, this};
            connect(action, &QAction::triggered, this, std::forward<decltype(slot)>(slot));
            auto* button = new QToolButton{this};
            button->setIconSize({24, 24});
            button->setDefaultAction(action);
            hLayout->addWidget(button);
        };
        toolButton(u"view-refresh"_s, tr("Reload scripts"), [this] {
            refreshScripts();
            for(const auto& name: registry_.scripts()) registry_.reload(name);
        });
        toolButton(u"folder-open"_s, tr("Open scripts folder"), [] {
            QDesktopServices::openUrl(QUrl::fromLocalFile(ScriptRegistry::dirPath()));
        });
        vLayout->addLayout(hLayout);
    }
    errorLabel->setWordWrap(true);
    errorLabel->setStyleSheet(u"color: red;"_s);
    errorLabel->hide();
    vLayout->addWidget(errorLabel);
    vLayout->addWidget(view);

    auto pushButton = new QPushButton{tr("Apply"), this};
    pushButton->setIcon(QIcon::fromTheme(u"dialog-ok-apply"_s));
    vLayout->addWidget(pushButton);
    connect(pushButton, &QPushButton::clicked, plugin, &Shapes::Plugin::finalizeShape);

    pushButton = new QPushButton{tr("Add New"), this};
    pushButton->setObjectName(u"pbAddNew"_s);
    pushButton->setIcon(QIcon::fromTheme(u"list-add"_s));
    vLayout->addWidget(pushButton);
    connect(pushButton, &QPushButton::clicked, this, [plugin] {
        plugin->finalizeShape();
        App::project().addShape(plugin->createShape());
    });

    pushButton = new QPushButton{u"Close"_s, this};
    pushButton->setObjectName(u"pbClose"_s);
    pushButton->setIcon(QIcon::fromTheme(u"window-close"_s));
    vLayout->addWidget(pushButton);

    view->setItemDelegate(new Delegate{model, view});
    view->setModel(model);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    connect(scriptBox, &QComboBox::currentTextChanged, this, [this](const QString& name) {
        if(resetFl || name.isEmpty()) return;
        for(auto* shape: selected(model->shapes)) shape->setScript(name);
        refresh();
    });

    connect(&registry_, &ScriptRegistry::scriptsChanged, this, &Editor::refreshScripts);
    connect(&registry_, &ScriptRegistry::scriptReloaded, this, [this](const QString& name) {
        for(auto* shape: model->shapes)
            if(shape->script() == name) shape->setScript(name); // перемердж параметров + redraw
        refresh();
    });
}

void Editor::refreshScripts() {
    const QString current = scriptBox->currentText();
    resetFl = true;
    scriptBox->clear();
    scriptBox->addItems(registry_.scripts());
    if(int idx = scriptBox->findText(current); idx >= 0) scriptBox->setCurrentIndex(idx);
    resetFl = false;
}

void Editor::applySpans() {
    view->clearSpans();
    for(int row = 1, rows = model->rowCount(); row < rows; ++row)
        view->setSpan(row, 0, 1, 2);
}

void Editor::updateError() {
    QString text;
    for(auto* shape: selected(model->shapes))
        if(!shape->lastError().isEmpty()) {
            text = shape->script() + u": "_s + shape->lastError();
            break;
        }
    errorLabel->setText(text);
    errorLabel->setVisible(!text.isEmpty());
}

void Editor::refresh() {
    model->sync(); // делает reset модели -> reset вью
    applySpans();
    updateError();
}

void Editor::add(Shapes::AbstractShape* shape) {
    model->shapes.emplace_back(static_cast<Shape*>(shape));
    refresh();
}

void Editor::remove(Shapes::AbstractShape* shape) {
    std::erase(model->shapes, static_cast<Shape*>(shape));
    refresh();
}

void Editor::updateData() {
    for(auto* shape: selected(model->shapes)) {
        if(!shape->script().isEmpty())
            if(int idx = scriptBox->findText(shape->script()); idx >= 0) {
                resetFl = true;
                scriptBox->setCurrentIndex(idx);
                resetFl = false;
            }
        break;
    }
    refresh();
}

} // namespace ShScript

#include "moc_editor.cpp"
