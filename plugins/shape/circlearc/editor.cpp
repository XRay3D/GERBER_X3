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
#include "shhandler.h"

#include <array>
#include <set>

Q_DECLARE_METATYPE(std::set<double>)

namespace ShArc {

//////////////////////////////////////////
/// \brief Model::Model
Model::Model(QObject* parent)
    : QAbstractTableModel{parent} { }

Model::~Model() { }

QVariant Model::data(const QModelIndex& index, int role) const {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    static const std::array getter{&QGraphicsItem::x, &QGraphicsItem::y};
    if(std::ranges::empty(sh)) return {};

    auto set = [index, &sh] {
        std::set<double> set;

        switch(index.row()) {
        case Shape::Point1:
        case Shape::Point2:
        case Shape::Center:
            for(auto* shape: sh) {
                auto tmp = (*shape->handlers[index.row()].*getter[index.column()])();
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
        case Shape::Angle1:
        case Shape::Angle2:
            for(auto* shape: sh) {
                auto tmp = shape->angle(index.row() - Shape::Angle1);
                set.emplace(tmp);
            }
            return set;
        default:
            return set;
        }
    };

    if(role == Qt::DisplayRole) {
        QString ret;
        for(auto val: set())
            ret += (ret.size() ? " | " : "") + QString::number(val);
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
        if(orientation == Qt::Vertical)
            return headerData_[section];
        else
            return section ? "Y" : "X";
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    static const std::array setter{&QGraphicsItem::setX, &QGraphicsItem::setY};

    if(role == Qt::EditRole) {
        if(std::ranges::empty(sh)) return {};

        double val = value.toDouble();

        switch(index.row()) {
        case Shape::Point1:
        case Shape::Point2:
        case Shape::Center:
            for(auto* shape: sh) {
                (*shape->handlers[index.row()].*setter[index.column()])(val);
                shape->currentHandler = shape->handlers[index.row()].get();
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
        case Shape::Angle1:
        case Shape::Angle2:
            for(auto* shape: sh)
                shape->setAngle(index.row() - Shape::Angle1, val);
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
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        qDebug(__FUNCTION__);
        dsbx = new QDoubleSpinBox{parent};

        switch(index.row()) {
        case Shape::Point1:
        case Shape::Point2:
        case Shape::Center:
            dsbx->setRange(-10000, +10000);
            break;
        case Shape::Radius:
        case Shape::Diameter:
            dsbx->setRange(0, 10000);
            break;
        case Shape::Angle1:
        case Shape::Angle2:
            dsbx->setRange(0, 360);
            break;
        }

        dsbx->setDecimals(3);
        connect(dsbx, &DoubleSpinBox::valueChanged, this, &Delegate::emitCommitData);
        connect(dsbx, &QObject::destroyed, this, [] { qDebug(__FUNCTION__); });
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
    view->setSpan(Shape::Angle1, 0, 1, 2);
    view->setSpan(Shape::Angle2, 0, 1, 2);
    view->setSpan(Shape::Diameter, 0, 1, 2);
    view->setSpan(Shape::Radius, 0, 1, 2);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    // view->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::SelectedClicked);
    //      connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
    //          view->edit(current);
    //      });
}

void Editor::addShape(Shape* shape) {
    model->shapes.emplace_back(shape);
    shape->model = model;
    view->reset();
}

} // namespace ShArc

#include "moc_editor.cpp"
