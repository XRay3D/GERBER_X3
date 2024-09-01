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

namespace ShRect {

//////////////////////////////////////////
/// \brief Model::Model
Model::Model(QObject* parent)
    : QAbstractTableModel{parent} { }

Model::~Model() { }

QVariant Model::data(const QModelIndex& index, int role) const {
    auto sh = shapes | std::views::filter([](Shape* sh) { return sh->isSelected(); });
    static const std::array getter{&QGraphicsItem::x, &QGraphicsItem::y};

    auto set = [&] {
        std::set<double> set;

        auto widthHeight = [&](auto get) {
            for(auto* shape: sh) {
                auto tmp = abs((*shape->handlers[Shape::Point1].*get)() - (*shape->handlers[Shape::Point3].*get)());
                set.emplace(tmp);
            }
            return set;
        };

        switch(index.row()) {
        case Shape::Center:
        case Shape::Point1:
        case Shape::Point2:
        case Shape::Point3:
        case Shape::Point4:
            for(auto* shape: sh) {
                auto tmp = (*shape->handlers[index.row()].*getter[index.column()])();
                set.emplace(tmp);
            }
            return set;
        case Shape::Width:
            return widthHeight(&QGraphicsItem::x);
        case Shape::Height:
            return widthHeight(&QGraphicsItem::y);
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
    static const std::array setter{&QGraphicsItem::setX, &QGraphicsItem::setY};

    if(role == Qt::EditRole) {
        if(std::ranges::empty(sh)) return {};

        double val = value.toDouble();

        auto widthHeight = [&sh, val](auto get, auto set) {
            for(auto* shape: sh) {
                (((*shape->handlers[Shape::Point1].*get)() - (*shape->handlers[Shape::Point3].*get)()) < 0
                        ? (*shape->handlers[Shape::Point3].*set)((*shape->handlers[Shape::Point1].*get)() + val)
                        : (*shape->handlers[Shape::Point3].*set)((*shape->handlers[Shape::Point1].*get)() - val));
                shape->currentHandler = shape->handlers[Shape::Point3].get();
                shape->redraw();
            }
        };

        switch(index.row()) {
        case Shape::Center:
        case Shape::Point1:
        case Shape::Point2:
        case Shape::Point3:
        case Shape::Point4:
            for(auto* shape: sh) {
                (*shape->handlers[index.row()].*setter[index.column()])(val);
                shape->currentHandler = shape->handlers[index.row()].get();
                shape->redraw();
            }
            break;
        case Shape::Width:
            widthHeight(&QGraphicsItem::x, &QGraphicsItem::setX);
            break;
        case Shape::Height:
            widthHeight(&QGraphicsItem::y, &QGraphicsItem::setY);
            break;
        }
        return true;
    }
    return {};
}

// class Delegate : public QStyledItemDelegate {
//     mutable QComboBox* cbx{};
//     mutable QString last;

// public:
//     Delegate(QObject* parent)
//         : QStyledItemDelegate{parent} { }
//     ~Delegate() override = default;

//    // QAbstractItemDelegate interface
//    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
//        qDebug(__FUNCTION__);
//        // if(cbx) return cbx;
//        cbx = new QComboBox{parent};
//        cbx->setEditable(true);
//        //        cbx->setCompleter(nullptr); //
//        connect(cbx->lineEdit(), &QLineEdit::textChanged, this, &Delegate::emitCommitData);
//        connect(cbx, &QObject::destroyed, this, [] { qDebug(__FUNCTION__); });
//        return cbx;
//    }

//    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
//        auto cbx = static_cast<QComboBox*>(editor);
//        for(auto val: index.data(Qt::EditRole).value<std::set<double>>())
//            cbx->addItem(QString::number(val), val);
//        last = cbx->currentText();
//        cbx->lineEdit()->setValidator(new QDoubleValidator{cbx});
//        cbx->installEventFilter(const_cast<Delegate*>(this));
//    }

//    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
//        if(last == cbx->currentText()) return;
//        auto cbx = static_cast<QComboBox*>(editor);
//        if(cbx->currentData().isValid()) model->setData(index, cbx->currentData().toDouble());
//        else model->setData(index, cbx->currentText().toDouble());
//    }

//    void emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }

//    // QObject interface
//    bool eventFilter(QObject* watched, QEvent* event) override {
//        if(cbx == watched && event->type() == QEvent::Show) {
//            if(cbx->count() > 1) cbx->showPopup();
//            cbx->lineEdit()->selectAll();
//        }
//        return QStyledItemDelegate::eventFilter(watched, event);
//    }
//};

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
    view->setSpan(Shape::Width, 0, 1, 2);
    view->setSpan(Shape::Height, 0, 1, 2);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    //    view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    //    connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
    //        view->edit(current);
    //    });
}

void Editor::addShape(Shape* shape) {
    model->shapes.emplace_back(shape);
    shape->model = model;
    view->reset();
}

} // namespace ShRect

#include "moc_editor.cpp"
