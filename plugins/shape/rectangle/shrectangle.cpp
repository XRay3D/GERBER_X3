// This is an open source non-commercial project. Dear PVS-Studio, please check it.
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
#include "shrectangle.h"
#include "graphicsview.h"
#include "shhandler.h"
#include <QIcon>
#include <ranges>

namespace Rectangle_ {

Shape::Shape(QPointF pt1, QPointF pt2) {
    paths_.resize(1);
    paths_.front().resize(5);
    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Shapes::Handle>(this, Shapes::Handle::Center));
    handlers.emplace_back(std::make_unique<Shapes::Handle>(this));
    handlers.emplace_back(std::make_unique<Shapes::Handle>(this));
    handlers.emplace_back(std::make_unique<Shapes::Handle>(this));
    handlers.emplace_back(std::make_unique<Shapes::Handle>(this));

    handlers[Point1]->setPos(pt1);
    handlers[Point3]->setPos(pt2);
    currentHandler = handlers[Point1].get();
    redraw();

    App::graphicsView().addItem(this);
}

void Shape::redraw() {

    auto updCenter = [this] { handlers[Center]->setPos(QLineF(handlers[Point1]->pos(), handlers[Point3]->pos()).center()); };
    auto updCorner = [this](int src, int p1, int p2) {
        handlers[p1]->setPos(handlers[src]->x(), currentHandler->y());
        handlers[p2]->setPos(currentHandler->x(), handlers[src]->y());
    };

    switch(handlers.indexOf(currentHandler)) {
    case Center: {
        QRectF rect(handlers[Point1]->pos(), handlers[Point3]->pos());
        rect.moveCenter(handlers[Center]->pos());
        auto center{handlers[Center]->pos()};
        handlers[Point1]->setPos(rect.topLeft());
        handlers[Point2]->setPos(rect.topRight());
        handlers[Point3]->setPos(rect.bottomRight());
        handlers[Point4]->setPos(rect.bottomLeft());
    } break;
    case Point1:
        updCorner(Point3, Point2, Point4), updCenter();
        break;
    case Point2:
        updCorner(Point4, Point1, Point3), updCenter();
        break;
    case Point3:
        updCorner(Point1, Point2, Point4), updCenter();
        break;
    case Point4:
        updCorner(Point2, Point1, Point3), updCenter();
        break;
    }

    paths_.front() = Path{
        handlers[Point1]->pos(),
        handlers[Point2]->pos(),
        handlers[Point3]->pos(),
        handlers[Point4]->pos(),
        handlers[Point1]->pos(),
    };

    if(Area(paths_.front()) < 0)
        ReversePath(paths_.front());
    shape_ = QPainterPath();
    shape_.addPolygon(paths_.front());
    setPos({1, 1}); // костыли    //update();
    setPos({0, 0});

    if(model)
        emit model->dataChanged(model->index(0, 0), model->index(1, 6));
}

QString Shape::name() const { return QObject::tr("Rectangle"); }

QIcon Shape::icon() const { return QIcon::fromTheme("draw-rectangle"); }

void Shape::setPt(const QPointF& pt) {
    handlers[Point3]->setPos(pt);
    updateOtherHandlers(handlers[Point3].get());
}
//////////////////////////////////////////
/// \brief Model::Model
/// \param parent
///
Model::Model(QObject* parent)
    : QAbstractTableModel{parent} { }

Model::~Model() { }

QVariant Model::data(const QModelIndex& index, int role) const {
    if(shape && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        switch(index.row()) {
        case 0:
            return abs(shape->handlers[Shape::Point1]->x() - shape->handlers[Shape::Point3]->x());
        case 1:
            return abs(shape->handlers[Shape::Point1]->y() - shape->handlers[Shape::Point3]->y());
        case 2:
            return index.column() ? shape->handlers[Shape::Center]->y() : shape->handlers[Shape::Center]->x();
        case 3:
            return index.column() ? shape->handlers[Shape::Point1]->y() : shape->handlers[Shape::Point1]->x();
        case 4:
            return index.column() ? shape->handlers[Shape::Point2]->y() : shape->handlers[Shape::Point2]->x();
        case 5:
            return index.column() ? shape->handlers[Shape::Point3]->y() : shape->handlers[Shape::Point3]->x();
        case 6:
            return index.column() ? shape->handlers[Shape::Point4]->y() : shape->handlers[Shape::Point4]->x();
        }
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

Qt::ItemFlags Model::flags(const QModelIndex& index) const {
    return Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(shape && role == Qt::EditRole) {
        double val = value.toDouble();
        switch(index.row()) {
        case 0:
            ((shape->handlers[Shape::Point1]->x() - shape->handlers[Shape::Point3]->x()) < 0 ? shape->handlers[Shape::Point3]->setX(shape->handlers[Shape::Point1]->x() + val) : shape->handlers[Shape::Point3]->setX(shape->handlers[Shape::Point1]->x() - val));
            break;
        case 1:
            ((shape->handlers[Shape::Point1]->y() - shape->handlers[Shape::Point3]->y()) < 0 ? shape->handlers[Shape::Point3]->setY(shape->handlers[Shape::Point1]->y() + val) : shape->handlers[Shape::Point3]->setY(shape->handlers[Shape::Point1]->y() - val));
            break;
        case 2:
            (index.column() ? shape->handlers[Shape::Center]->setY(val) : shape->handlers[Shape::Center]->setX(val));
            break;
        case 3:
            (index.column() ? shape->handlers[Shape::Point1]->setY(val) : shape->handlers[Shape::Point1]->setX(val));
            break;
        case 4:
            (index.column() ? shape->handlers[Shape::Point2]->setY(val) : shape->handlers[Shape::Point2]->setX(val));
            break;
        case 5:
            (index.column() ? shape->handlers[Shape::Point3]->setY(val) : shape->handlers[Shape::Point3]->setX(val));
            break;
        case 6:
            (index.column() ? shape->handlers[Shape::Point4]->setY(val) : shape->handlers[Shape::Point4]->setX(val));
            break;
        }
        shape->redraw();
        return true;
    }
    return {};
}

Editor::Editor(Plugin* plugin)
    : /* QWidget {parent}*/ view{new QTableView{this}}
    , model{new Model{view}}
    , plugin{plugin} {
    setWindowTitle("Rectangle");

    auto vLayout = new QVBoxLayout{this};
    vLayout->setContentsMargins(6, 6, 6, 6);
    vLayout->setSpacing(6);
    {
        auto hLayout = new QHBoxLayout;
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(6);
        for(int i = 0; i < 6; ++i) {
            auto action = new QAction{this};
            action->setIcon(QIcon::fromTheme("draw-rectangle"));
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

    view->setModel(model);
    view->setSpan(0, 0, 1, 2);
    view->setSpan(1, 0, 1, 2);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

} // namespace Rectangle_

#include "moc_shrectangle.cpp"
