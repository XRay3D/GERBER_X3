/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
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

namespace ShCirc {

//////////////////////////////////////////
/// \brief Model::Model
Model::Model(QObject* parent)
    : QAbstractTableModel{parent} { }

Model::~Model() { }

QVariant Model::data(const QModelIndex& index, int role) const {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    static const std::array getter{&Shapes::Handle::x, &Shapes::Handle::y};

    auto set = [&] {
        std::set<double> set;

        switch(index.row()) {
        case Shape::Center:
        case Shape::Point1:
            for(auto* shape: sh) {
                auto tmp = (shape->handles[index.row()].*getter[index.column()])();
                set.emplace(tmp);
            }
            return set;
        case Shape::Radius:
            for(auto* shape: sh) {
                auto tmp = shape->radius();
                set.emplace(tmp);
            }
            return set;
        case Shape::Diameter:
            for(auto* shape: sh) {
                auto tmp = shape->radius() * 2;
                set.emplace(tmp);
            }
            return set;
        default: return set;
        }
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
    if(role == Qt::DisplayRole) {
        if(orientation == Qt::Vertical)
            return headerData_[section];
        else
            return section ? "Y" : "X";
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    static const std::array setter{&Shapes::Handle::setX, &Shapes::Handle::setY};

    if(role == Qt::EditRole) {
        if(std::ranges::empty(sh)) return {};

        double val = value.toDouble();

        switch(index.row()) {
        case Shape::Center:
        case Shape::Point1:
            for(auto* shape: sh) {
                (shape->handles[index.row()].*setter[index.column()])(val);
                shape->curHandle = shape->handles.begin() + index.row();
                shape->redraw();
            }
            break;
        case Shape::Radius:
            for(auto* shape: sh)
                shape->setRadius(val);
            break;
        case Shape::Diameter:
            for(auto* shape: sh)
                shape->setRadius(val / 2);
            break;
        }
        return true;
    }
    return {};
}

class Delegate : public QStyledItemDelegate {
    mutable QDoubleSpinBox* dsbx{};
    mutable double last;

public:
    Delegate(QObject* parent)
        : QStyledItemDelegate{parent} { }
    ~Delegate() override = default;

    // QAbstractItemDelegate interface
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const override {
        qDebug(__FUNCTION__);
        dsbx = new QDoubleSpinBox{parent};
        index.row() >= Shape::PtCount
            ? dsbx->setRange(0, +10000)
            : dsbx->setRange(-10000, +10000);
        dsbx->setDecimals(3);
        connect(dsbx, &DoubleSpinBox::valueChanged, this, &Delegate::emitCommitData);
        connect(dsbx, &QObject::destroyed, this, [] { qDebug(__FUNCTION__); });
        return dsbx;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        dsbx = static_cast<QDoubleSpinBox*>(editor);
        dsbx->setValue(last = *index.data(Qt::EditRole).value<std::set<double>>().begin());
        last = dsbx->value();
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        dsbx = static_cast<QDoubleSpinBox*>(editor);
        if(qFuzzyCompare(last, dsbx->value())) return;
        model->setData(index, dsbx->value());
    }

    void emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }
};

//////////////////////////////////////////
/// \brief Editor::Editor
Editor::Editor(Shapes::Plugin* plugin)
    : /* Shapes::Editor{parent}*/ view{new QTableView{this}}
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
    connect(pushButton, &QPushButton::clicked, this, [plugin] {
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
    view->setSpan(Shape::Radius, 0, 1, 2);
    view->setSpan(Shape::Diameter, 0, 1, 2);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    // view->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::SelectedClicked);
    //      connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
    //          view->edit(current);
    //      });
}

void Editor::add(Shapes::AbstractShape* shape) {
    model->shapes.emplace_back(static_cast<Shape*>(shape));
    view->reset();
}

void Editor::remove(Shapes::AbstractShape* shape) {
    std::erase(model->shapes, static_cast<Shape*>(shape));
    view->reset();
}

} // namespace ShCirc

#include "moc_editor.cpp"
