// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "editor.h"
#include "doublespinbox.h"
#include "shape.h"
#include "shhandler.h"

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
        sise = std::max(sise, shape->handlers.size());
    return sise;
}

QVariant Model::data(const QModelIndex& index, int role) const {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    static const std::array getter{&QGraphicsItem::x, &QGraphicsItem::y};

    auto set = [&] {
        std::set<double> set;
        for(auto* shape: sh) {
            if(shape->handlers.size() <= index.row())
                continue;
            auto tmp = (*shape->handlers[index.row()].*getter[index.column()])();
            set.emplace(tmp);
        }
        return set;
    };

    if(role == Qt::DisplayRole) {
        if(std::ranges::empty(sh)) return {};
        QString ret;
        for(auto val: set())
            ret += (ret.size() ? " | " : "") + QString::number(val);
        return ret;
    }
    if(role == Qt::EditRole) {
        if(std::ranges::empty(sh)) return {};
        return QVariant::fromValue(set());
    }
    if(role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;

    return {};
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return section ? "Y" : "X";
    if(role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags;
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    if(std::ranges::empty(sh)) return {};
    for(auto* shape: sh) {
        if(shape->handlers.size() <= index.row())
            continue;
        if(shape->handlers[index.row()]->hType() == Handle::Corner)
            flags = Qt::ItemIsEnabled | Qt::ItemIsEditable;
        break;
    }
    return flags;
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    static const std::array setter{&QGraphicsItem::setX, &QGraphicsItem::setY};

    if(role == Qt::EditRole) {
        if(std::ranges::empty(sh)) return {};

        double val = value.toDouble();

        for(auto* shape: sh) {
            if(shape->handlers.size() <= index.row())
                continue;
            (*shape->handlers[index.row()].*setter[index.column()])(val);
            shape->currentHandler = shape->handlers[index.row()].get();
            shape->redraw();
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
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
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

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        if(qFuzzyCompare(last, dsbx->value())) return;
        model->setData(index, dsbx->value());
    }

    void emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }
};

//////////////////////////////////////////
/// \brief Editor::Editor
Editor::Editor(Plugin* plugin)
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
        for(int i = 0; i < 6; ++i) {
            auto action = new QAction{this};
            action->setIcon(plugin->icon());
            action->setCheckable(true);
            auto toolButton = new QToolButton{this};
            toolButton->setIconSize({24, 24});
            toolButton->setDefaultAction(action);
            actionGroup.addAction(action);
            hLayout->addWidget(toolButton);
        }

        vLayout->addLayout(hLayout);
        hLayout->stretch(5);
    }
    vLayout->addWidget(view);

    auto pushButton = new QPushButton{tr("Apply"), this};
    pushButton->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    vLayout->addWidget(pushButton);
    connect(pushButton, &QPushButton::clicked, plugin, &Shapes::Plugin::finalizeShape);

    pushButton = new QPushButton{tr("Add New"), this};
    pushButton->setObjectName("pbAddNew");
    pushButton->setIcon(QIcon::fromTheme("list-add"));
    vLayout->addWidget(pushButton);
    connect(pushButton, &QPushButton::clicked, this, [plugin, this] {
        plugin->finalizeShape();
        App::project().addShape(plugin->createShape());
    });

    pushButton = new QPushButton{"Close", this};
    pushButton->setObjectName("pbClose");
    pushButton->setIcon(QIcon::fromTheme("window-close"));
    vLayout->addWidget(pushButton);

    vLayout->setSpacing(6);

    view->setItemDelegate(new Delegate{view});

    view->setModel(model);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    //    connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
    //        view->edit(current);
    //    });
}

void Editor::addShape(Shape* shape) {
    model->shapes.emplace_back(shape);
    shape->model = model;
    view->reset();
}

} // namespace ShPoly

#include "moc_editor.cpp"
