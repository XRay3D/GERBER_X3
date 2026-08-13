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
#include "editor.h"
#include "doublespinbox.h"
#include "shape.h"

#include <array>
#include <set>

Q_DECLARE_METATYPE(std::set<double>)
using Shapes::Handle;

namespace ShPoly {

//////////////////////////////////////////
/// \brief Model::Model
Model::Model(QObject* parent)
    : QAbstractTableModel{parent} { }

Model::~Model() { }

int Model::rowCount(const QModelIndex&) const {
    size_t sise{};
    for(auto&& shape: shapes)
        sise = std::max(sise, shape->handles.size());
    return sise;
}

QVariant Model::data(const QModelIndex& index, int role) const {
    auto sh = shapes | v::filter([](Shape* sh) { return sh->isSelected(); });
    static const std::array getter{&Handle::x, &Handle::y};

    auto set = [&] {
        std::set<double> set;
        for(auto* shape: sh) {
            if(shape->handles.size() <= size_t(index.row()))
                continue;
            auto tmp = (shape->handles[index.row()].*getter[index.column()])();
            set.emplace(tmp);
        }
        return set;
    };

    if(role == Qt::DisplayRole) {
        if(r::empty(sh)) return {};
        QString ret;
        for(auto val: set())
            ret += (ret.size() ? u" | " : u"") + QString::number(val);
        return ret;
    }
    if(role == Qt::EditRole) {
        if(r::empty(sh)) return {};
        return QVariant::fromValue(set());
    }
    if(role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;

    return {};
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return section ? u"Y"_s : u"X"_s;
    if(role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags;
    auto sh = shapes | v::filter([](Shape* sh) { return sh->isSelected(); });
    if(r::empty(sh)) return {};
    for(auto* shape: sh) {
        if(shape->handles.size() <= size_t(index.row()))
            continue;
        if(shape->handles[index.row()].type() == Handle::Corner)
            flags = Qt::ItemIsEnabled | Qt::ItemIsEditable;
        break;
    }
    return flags;
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    auto sh = shapes | v::filter([](Shape* sh) { return sh->isSelected(); });
    static const std::array setter{&Handle::setX, &Handle::setY};

    if(role == Qt::EditRole) {
        if(r::empty(sh)) return {};

        double val = value.toDouble();

        for(auto* shape: sh) {
            if(shape->handles.size() <= size_t(index.row())) continue;
            (shape->handles[index.row()].*setter[index.column()])(val);
            // курсорная ручка — чтобы rebuild подтянул соседние средние ручки
            shape->curHandle = shape->handles.data() + index.row();
            shape->redraw();
            shape->curHandle = {};
        }

        return true;
    }
    return {};
}

class Delegate : public QStyledItemDelegate {
    mutable DoubleSpinBox* dsbx{};
    mutable double last;

public:
    Delegate(QObject* parent)
        : QStyledItemDelegate{parent} { }
    ~Delegate() override = default;

    // QAbstractItemDelegate interface
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override {
        qDebug(__FUNCTION__);
        dsbx = new DoubleSpinBox{parent};
        connect(dsbx, &DoubleSpinBox::valueChanged, this, &Delegate::emitCommitData);
        connect(dsbx, &QObject::destroyed, this, [] { qDebug(__FUNCTION__); });
        return dsbx;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        dsbx = static_cast<DoubleSpinBox*>(editor);
        dsbx->setRange(-1000, +1000);
        auto set = index.data(Qt::EditRole).value<std::set<double>>();
        if(set.size()) dsbx->setValue(last = *set.begin());
    }

    void setModelData(QWidget* /*editor*/, QAbstractItemModel* model, const QModelIndex& index) const override {
        if(qFuzzyCompare(last, dsbx->value())) return;
        model->setData(index, dsbx->value());
    }

    void emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }
};

//////////////////////////////////////////
/// \brief Editor::Editor
Editor::Editor(Shapes::Plugin* plugin)
    : /* QWidget {parent}*/ view{new QTableView{this}}
    , model{new Model{view}}
    , plugin{plugin} {
    setWindowTitle(plugin->name());

    auto vLayout = new QVBoxLayout{this};
    vLayout->setContentsMargins(6, 6, 6, 6);
    vLayout->setSpacing(6);
    {
        auto hLayout = new QHBoxLayout;
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(6);

        closedAction = new QAction{QIcon::fromTheme(u"draw-polygon"_s), tr("Closed"), this};
        closedAction->setCheckable(true);
        closedAction->setToolTip(tr("Close the polyline"));
        connect(closedAction, &QAction::toggled, this, [this](bool checked) {
            if(resetFl) return;
            for(auto* shape: model->shapes)
                if(shape->isSelected()) shape->setClosed(checked);
        });
        auto toolButton = new QToolButton{this};
        toolButton->setIconSize({24, 24});
        toolButton->setDefaultAction(closedAction);
        hLayout->addWidget(toolButton);

        auto modeButton = [&](const QString& icon, const QString& text, bool checked) {
            auto action = new QAction{QIcon::fromTheme(icon), text, this};
            action->setCheckable(true);
            action->setChecked(checked);
            actionGroup.addAction(action);
            auto tb = new QToolButton{this};
            tb->setIconSize({24, 24});
            tb->setDefaultAction(action);
            hLayout->addWidget(tb);
            return action;
        };
        modeButton(u"draw-line"_s, tr("Drag middle handle: split segment"), true);
        arcAction = modeButton(u"draw-ellipse-arc"_s, tr("Drag middle handle: bend segment into arc"), false);

        hLayout->addStretch();
        vLayout->addLayout(hLayout);
    }
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

    vLayout->setSpacing(6);

    view->setItemDelegate(new Delegate{view});

    view->setModel(model);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    // connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
    // view->edit(current);
    // });
}

void Editor::add(Shapes::AbstractShape* shape) {
    model->shapes.emplace_back(static_cast<Shape*>(shape));
    view->reset();
}

void Editor::remove(Shapes::AbstractShape* shape) {
    std::erase(model->shapes, static_cast<Shape*>(shape));
    view->reset();
}

void Editor::updateData() {
    view->reset();
    for(auto* shape: model->shapes)
        if(shape->isSelected()) {
            resetFl = true;
            closedAction->setChecked(shape->isClosed());
            resetFl = false;
            break;
        }
}

} // namespace ShPoly

#include "moc_editor.cpp"
