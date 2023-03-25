// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_formsutil.h"

#include "app.h"
#include "fileifce.h"
#include "gi_error.h"
#include "graphicsview.h"
#include "project.h"

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QIcon>
#include <QPainter>
#include <QProgressDialog>
#include <QPushButton>
#include <QTableView>
#include <QtWidgets>

#include <condition_variable>
#include <execution>
#include <mutex>
#include <ranges>
#include <sstream>

static const int gcpId = qRegisterMetaType<GCode::GCodeParams>("GCode::GCodeParams");
static const int GCodeFileId = qRegisterMetaType<GCode::File*>("GCode::File*");

inline QIcon errorIcon(const QPainterPath& path) {

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
                return QObject::tr("Position");
            } else {
                return QObject::tr("Area mm²");
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

GcFormBase::GcFormBase(GCodePlugin* plugin, GCode::Creator* tpc, QWidget* parent)
    : QWidget(parent)
    , plugin {plugin}
    , gcCreator {tpc}
    , progressDialog(new QProgressDialog(this)) {

    auto vLayout = new QVBoxLayout(this);
    vLayout->addWidget(ctrWidget = new QWidget(this));
    vLayout->addWidget(errWidget = new QWidget(this));
    vLayout->setContentsMargins(0, 0, 0, 0);

    { // Creator
        content = new QWidget(ctrWidget);

        dsbxDepth = new DepthForm(ctrWidget);
        dsbxDepth->setObjectName("dsbxDepth");

        leName = new QLineEdit(ctrWidget);
        leName->setObjectName("leName");

        pbClose = new QPushButton(tr("Close"), ctrWidget);
        pbClose->setIcon(QIcon::fromTheme("window-close"));
        pbClose->setObjectName("pbClose");

        pbCreate = new QPushButton(tr("Create"), ctrWidget);
        pbCreate->setIcon(QIcon::fromTheme("document-export"));
        pbCreate->setObjectName("pbCreate");
        connect(pbCreate, &QPushButton::clicked, this, &GcFormBase::createFile);

        auto line = [this] {
            auto line = new QFrame(ctrWidget);
            line->setFrameShadow(QFrame::Plain);
            line->setFrameShape(QFrame::HLine);
            return line;
        };

        grid = new QGridLayout(ctrWidget);
        grid->setContentsMargins(6, 6, 6, 6);
        grid->setSpacing(6);

        int row {};
        constexpr int rowSpan {1}, columnSpan {2};
        // clang-format off
        grid->addWidget(dsbxDepth,                       row, 0, rowSpan, columnSpan); // row 0
        grid->addWidget(line(),                        ++row, 0, rowSpan, columnSpan); // row 1
        grid->addWidget(content,                       ++row, 0, rowSpan, columnSpan); // row 2
        grid->addWidget(line(),                        ++row, 0, rowSpan, columnSpan); // row 3
        grid->addWidget(new QLabel(tr("Name:"), this), ++row, 0);                      // row 4
        grid->addWidget(leName,                          row, 1);                      // row 4
        grid->addWidget(pbCreate,                      ++row, 0, rowSpan, columnSpan); // row 5
        grid->addWidget(pbClose,                       ++row, 0, rowSpan, columnSpan); // row 6
        grid->addWidget(new QWidget(this),             ++row, 0, rowSpan, columnSpan); // row 7
        // clang-format on
        grid->setRowStretch(row, 1);
    }
    { // On Error
        auto grid = new QVBoxLayout(errWidget);
        grid->addWidget(errTable = new TableView(errWidget));
        grid->addWidget(errBtnBox = new QDialogButtonBox(errWidget));

        errBtnBox->setObjectName(QString::fromUtf8("errBtnBox"));
        errBtnBox->setOrientation(Qt::Horizontal);
        errBtnBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

        errBtnBox->button(QDialogButtonBox::Ok)->setText(tr("Continue"));
        errBtnBox->button(QDialogButtonBox::Cancel)->setText(tr("Break"));

        connect(errBtnBox->button(QDialogButtonBox::Ok),
            &QAbstractButton::clicked, this, &GcFormBase::errContinue);
        connect(errBtnBox->button(QDialogButtonBox::Cancel),
            &QAbstractButton::clicked, this, &GcFormBase::errBreak);
        errWidget->setVisible({});
    }

    progressDialog->setMinimumDuration(100);
    progressDialog->setModal(true);
    progressDialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);
    progressDialog->reset();
    connect(progressDialog, &QProgressDialog::canceled, this, &GcFormBase::cancel);

    if (!gcCreator)
        return;

    connect(this, &GcFormBase::createToolpath, this, &GcFormBase::startProgress);
    connect(this, &GcFormBase::createToolpath, gcCreator, &GCode::Creator::createGc);

    connect(gcCreator, &GCode::Creator::canceled, this, &GcFormBase::stopProgress);
    connect(gcCreator, &GCode::Creator::errorOccurred, this, &GcFormBase::errorHandler);
    connect(gcCreator, &GCode::Creator::fileReady, this, &GcFormBase::fileHandler);

    gcCreator->moveToThread(&thread);
    connect(&thread, &QThread::finished, gcCreator, &QObject::deleteLater);
    thread.start(QThread::LowPriority /*HighestPriority*/);
}

GcFormBase::~GcFormBase() {
    if (errWidget->isVisible())
        errBreak();
    thread.quit();
    thread.wait();
}

void GcFormBase::fileHandler(GCode::File* file) {
    qDebug() << __FUNCTION__ << file;
    if (--fileCount == 0)
        cancel();

    if (file == nullptr) {
        QMessageBox::information(this, tr("Warning"), tr("The tool doesn`t fit in the Working items!"));
        return;
    }

    file->setFileName(fileName_ + "_" + file->name());
    file->setSide(boardSide);
    if (fileId > -1) {
        exit(-123456);
        //        App::project()->reload(fileId, file);
        //        editMode_ = false;
        //        fileId = -1;
    } else {
        App::project()->addFile(file);
    }
}

void GcFormBase::timerEvent(QTimerEvent* event) {
    if (event->timerId() == progressTimerId && progressDialog && gcCreator) {
        const auto [max, val] = gcCreator->getProgress();
        progressDialog->setMaximum(max);
        progressDialog->setValue(val);
        progressDialog->setLabelText(gcCreator->msg);
    }
}

void GcFormBase::addUsedGi(GraphicsItem* gi) {
    if (gi->file()) {
        //        GCode::File const* file = gi->file();
        //        if (file->type() == FileType::Gerber) {
        // #ifdef GBR_
        //            usedItems_[{file->id(), reinterpret_cast<const Gerber::File*>(file)->itemsType()}].push_back(gi->id());
        // #endif
        //        } else {
        //            usedItems_[{file->id(), -1}].push_back(gi->id());
        //        }
    }
}

void GcFormBase::cancel() {
    gcCreator->continueCalc({});
    stopProgress();
}

void GcFormBase::errorHandler(int) {
    qDebug(__FUNCTION__);

    if (gcCreator->items.empty())
        return;

    gcCreator->checkMillingFl = false;
    stopProgress();

    ctrWidget->setVisible(false);
    errWidget->setVisible(true);

    std::ranges::for_each(gcCreator->items, [](auto i) { App::graphicsView()->addItem(i); });

    errTable->setModel(new ErrorModel(std::move(gcCreator->items), errTable));
    errTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    errTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    App::graphicsView()->startUpdateTimer(32);
}

void GcFormBase::errContinue() {
    qDebug(__FUNCTION__);
    App::graphicsView()->stopUpdateTimer();

    delete errTable->model();

    ctrWidget->setVisible(true);
    errWidget->setVisible({});

    startProgress();
    gcCreator->continueCalc(true);
}

void GcFormBase::errBreak() {
    qDebug(__FUNCTION__);
    App::graphicsView()->stopUpdateTimer();

    delete errTable->model();

    ctrWidget->setVisible(true);
    errWidget->setVisible({});

    gcCreator->continueCalc(false);
}

void GcFormBase::startProgress() {
    qDebug(__FUNCTION__);
    if (!fileCount)
        fileCount = 1;
    gcCreator->msg = fileName_;
    progressDialog->setLabelText(gcCreator->msg);
    progressTimerId = startTimer(100);
}

void GcFormBase::stopProgress() {
    qDebug() << __FUNCTION__ << gcCreator->checkMillingFl;
    if (gcCreator->checkMillingFl)
        return;
    killTimer(progressTimerId);
    progressTimerId = 0;
    progressDialog->reset();
    progressDialog->hide();
}

#include "moc_gc_formsutil.cpp"
