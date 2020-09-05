#include "errordialog.h"
#include "app.h"
#include "gi/erroritem.h"
#include "graphicsview.h"
#include "scene.h"
#include "ui_errordialog.h"
#include <QHeaderView>
#include <QPushButton>
#include <QTableView>

Q_DECLARE_METATYPE(ErrorItem*)

//int id = qRegisterMetaType<ErrorItem*>();

enum { IconSize = 32 };

QIcon errorIcon(const QPainterPath& path)
{

    const QRectF rect = path.boundingRect();

    qreal scale = static_cast<double>(IconSize) / qMax(rect.width(), rect.height());

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
    painter.setBrush(Qt::black);
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    painter.drawPath(path);
    return QIcon(pixmap);
}

class ErrorModel : public QAbstractTableModel {
    QVector<ErrorItem*> items;

public:
    ErrorModel(QVector<ErrorItem*> items, QObject* parent = nullptr)
        : QAbstractTableModel(parent)
        , items(items)
    {
    }
    virtual ~ErrorModel()
    {
        qDeleteAll(items);
    }

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex&) const override { return items.size(); }
    int columnCount(const QModelIndex&) const override { return 2; }
    QVariant data(const QModelIndex& index, int role) const override
    {
        switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0) {
                auto pos { items[index.row()]->boundingRect().center() };
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
            return {};
        }
        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == 0) {
                return tr("Position");
            } else {
                return tr("Area mm²");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void updateScene()
    {
        QRectF rect;
        for (auto item : items)
            rect = rect.united(item->boundingRect());
        App::graphicsView()->fitInView(rect, Qt::KeepAspectRatio);
    }
};

class TableView : public QTableView {
    //    Q_OBJECT
public:
    TableView(QWidget* parent)
        : QTableView(parent)
    {
        horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setIconSize({ IconSize, IconSize });
    }
    virtual ~TableView() { }

    // QAbstractItemView interface
protected slots:
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override
    {
        QTableView::selectionChanged(selected, deselected);
        for (auto& var : selected.indexes()) {
            var.data(Qt::UserRole).value<ErrorItem*>()->setSelected(true);
        }
        for (auto& var : deselected.indexes()) {
            var.data(Qt::UserRole).value<ErrorItem*>()->setSelected(false);
        }
        static_cast<ErrorModel*>(model())->updateScene();
    }
    void currentChanged(const QModelIndex& current, const QModelIndex& previous) override
    {
        QTableView::currentChanged(current, previous);
        if (current.isValid()) {
            auto item = current.data(Qt::UserRole).value<ErrorItem*>();
            item->setSelected(true);
        }
        if (previous.isValid() && !selectionModel()->isSelected(previous)) {
            previous.data(Qt::UserRole).value<ErrorItem*>()->setSelected(false);
        }
        static_cast<ErrorModel*>(model())->updateScene();
    }

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override
    {
        QTableView::showEvent(event);
        resizeRowsToContents();
    }
};

ErrorDialog::ErrorDialog(const QVector<ErrorItem*>& items, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
    ui->verticalLayout->insertWidget(0, table = new TableView(this));
    table->setModel(new ErrorModel(items, table));
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    for (auto item : items) {
        App::scene()->addItem(item);
        item->setZValue(std::numeric_limits<double>::max());
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Continue"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Break"));
}

ErrorDialog::~ErrorDialog()
{
    delete table->model();
    delete ui;
}
