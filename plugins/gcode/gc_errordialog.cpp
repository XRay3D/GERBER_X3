// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/

#include "gc_errordialog.h"

#include "gi_error.h"
#include "graphicsview.h"
#include "mainwindow.h"
#include "scene.h"

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QIcon>
#include <QPainter>
#include <QPushButton>
#include <QTableView>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>

Q_DECLARE_METATYPE(GiError*)

// enum { IconSize = 32 };

QIcon errorIcon(const QPainterPath& path) {

    const QRectF rect = path.boundingRect();

    double scale = static_cast<double>(IconSize) / std::max(rect.width(), rect.height());

    double ky = rect.bottom() * scale;
    double kx = rect.left() * scale;
    if (rect.width() > rect.height())
        ky += (static_cast<double>(IconSize) - rect.height() * scale) / 2;
    else
        kx -= (static_cast<double>(IconSize) - rect.width() * scale) / 2;

    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(App::settings().theme() > LightRed ? Qt::white : Qt::black);
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    painter.drawPath(path);
    return pixmap;
}

class ErrorModel : public QAbstractTableModel {
    mvector<GiError*> items;

public:
    ErrorModel(mvector<GiError*>&& items, QObject* parent = nullptr)
        : QAbstractTableModel(parent)
        , items(std::move(items)) {
    }
    virtual ~ErrorModel() { qDeleteAll(items); }

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex&) const override { return static_cast<int>(items.size()); }
    int columnCount(const QModelIndex&) const override { return 2; }
    QVariant data(const QModelIndex& index, int role) const override {
        switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0) {
                auto pos {items[index.row()]->boundingRect().center()};
                return QString("X = %1\nY = %2").arg(pos.x()).arg(pos.y());
            }
            return items[index.row()]->area();
        case Qt::UserRole:
            return QVariant::fromValue(items[index.row()]);
        case Qt::DecorationRole:
            if (index.column() == 0)
                return errorIcon(items[index.row()]->shape());
            return {};
        case Qt::TextAlignmentRole:
            if (index.column() == 0)
                return Qt::AlignVCenter;
            return Qt::AlignCenter;
        default:
            break;
        }
        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == 0) {
                return tr("Position");
            } else {
                return tr("Area mm²");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void updateScene() {
        QRectF rect;
        for (auto item : items)
            if (item->isSelected())
                rect = rect.united(item->boundingRect());
        App::graphicsView()->fitInView(rect);
        //        App::graphicsView()->zoomOut();
    }
};

class TableView : public QTableView {
    //    Q_OBJECT
public:
    TableView(QWidget* parent)
        : QTableView(parent) {
        horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setIconSize({IconSize, IconSize});
    }
    virtual ~TableView() { }

    // QAbstractItemView interface
protected slots:
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override {
        QTableView::selectionChanged(selected, deselected);
        for (auto& var : selected.indexes()) {
            var.data(Qt::UserRole).value<GiError*>()->setSelected(true);
        }
        for (auto& var : deselected.indexes()) {
            var.data(Qt::UserRole).value<GiError*>()->setSelected(false);
        }
        static_cast<ErrorModel*>(model())->updateScene();
    }

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override {
        QTableView::showEvent(event);
        resizeRowsToContents();
    }
};

void ErrorDialog::setupUi(QDialog* ErrorDialog) {
    if (ErrorDialog->objectName().isEmpty())
        ErrorDialog->setObjectName(QString::fromUtf8("ErrorDialog"));
    ErrorDialog->resize(471, 605);
    verticalLayout = new QVBoxLayout(ErrorDialog);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(6, 6, 6, 6);
    buttonBox = new QDialogButtonBox(ErrorDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    verticalLayout->addWidget(buttonBox);

    retranslateUi(ErrorDialog);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, ErrorDialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, ErrorDialog, &QDialog::reject);

    QMetaObject::connectSlotsByName(ErrorDialog);
}

void ErrorDialog::retranslateUi(QDialog* ErrorDialog) {
    ErrorDialog->setWindowTitle(QCoreApplication::translate("ErrorDialog", "Uncut places:", nullptr));
}

ErrorDialog::ErrorDialog(mvector<GiError*>&& items, QWidget* parent)
    : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint) {

    //    mvector<GiError*> tmp;

    //    for (auto item : items) {
    //        App::graphicsView()->scene()->addItem(item);
    //        item->setZValue(std::numeric_limits<double>::max());

    //        mvector<QGraphicsItem*> ci;

    //        {
    //            auto tmp = App::graphicsView()->scene()->collidingItems(item);
    //            ci.reserve(tmp.size());
    //            ci.insert(ci.begin(), tmp.begin(), tmp.end());
    //        }
    //        std::erase_if(ci, [](QGraphicsItem* item) { return item->type() != int(GiType::DataSolid) && item->type() != int(GiType::DataPath) && item->type() != int(GiType::Drill); });
    //        qDebug() << __FUNCTION__ << ci.size();
    //        if (ci.size() >= 2) {
    //            tmp.emplace_back(item);
    //        } else
    //            delete item;
    //    }
    //    qDebug() << __FUNCTION__ << tmp.size();
    //    items = tmp;

    setupUi(this);

    for (auto scene {App::graphicsView()->scene()}; auto item : items)
        scene->addItem(item);

    verticalLayout->insertWidget(0, table = new TableView(this));
    table->setModel(new ErrorModel(std::move(items), table));
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Continue"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Break"));
    setGeometry(parent->normalGeometry());
    startTimer(32);
}

ErrorDialog::~ErrorDialog() { }

void ErrorDialog::timerEvent(QTimerEvent* event) {
    if (!table->model()->rowCount())
        accept();
    App::graphicsView()->scene()->update();
}

void ErrorDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    setModal(false);                       // FIXME Dialog specific
    setAttribute(Qt::WA_ShowModal, false); // FIXME Set the WA_ShowModal property
    setWindowModality(Qt::NonModal);       // FIXME Set the blocking mode
    //    if (!table->model()->rowCount())
    //        accept();
}
