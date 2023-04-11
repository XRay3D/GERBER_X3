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
#include "gc_baseform.h"

#include "abstract_file.h"
#include "app.h"
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

// static const int id1 = qRegisterMetaType<GCode::Params>("GCode::Params");
// static const int id2 = qRegisterMetaType<GCode::Params&>("GCode::Params&");
static const int id3 = qRegisterMetaType<GCode::File*>("GCode::File*");

namespace GCode {

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
        App::graphicsView().fitInView(rect);
        //        App::graphicsView().zoomOut();
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

BaseForm::BaseForm(Plugin* plugin, Creator* tpc, QWidget* parent)
    : QWidget(parent)
    //, plugin {plugin}
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
        connect(pbCreate, &QPushButton::clicked, this, &BaseForm::computePaths);

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
            &QAbstractButton::clicked, this, &BaseForm::errContinue);
        connect(errBtnBox->button(QDialogButtonBox::Cancel),
            &QAbstractButton::clicked, this, &BaseForm::errBreak);
        errWidget->setVisible({});
    }

    progressDialog->setMinimumDuration(100);
    progressDialog->setModal(true);
    progressDialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);
    progressDialog->reset();
    connect(progressDialog, &QProgressDialog::canceled, this, &BaseForm::cancel);
    setCreator(tpc);
}

BaseForm::~BaseForm() {
    if (errWidget->isVisible())
        errBreak();
    thread.quit();
    thread.wait();
}

void BaseForm::setCreator(Creator* newCreator) {
    qDebug() << __FUNCTION__ << creator_ << newCreator;

    if (creator_ != newCreator && newCreator) {
        qDebug(__FUNCTION__);
        if (thread.isRunning()) {
            thread.quit();
            thread.wait();
        }
        creator_ = newCreator;
        connect(this, &BaseForm::createToolpath, this, &BaseForm::startProgress);
        connect(this, &BaseForm::createToolpath, creator_, &Creator::createGc, Qt::QueuedConnection);
        connect(creator_, &Creator::canceled, this, &BaseForm::stopProgress);
        connect(creator_, &Creator::errorOccurred, this, &BaseForm::errorHandler);
        connect(creator_, &Creator::fileReady, this, &BaseForm::fileHandler);
        creator_->moveToThread(&thread);
        connect(&thread, &QThread::finished, creator_, &QObject::deleteLater);
        thread.start(QThread::LowPriority /*HighestPriority*/);
    } else if (creator_ && !newCreator) {
        thread.quit();
        thread.wait();
        creator_ = nullptr;
    }
}

void BaseForm::fileHandler(File* file) {
    qDebug() << __FUNCTION__ << file;
    if (--fileCount == 0)
        cancel();

    if (file == nullptr) {
        auto message = tr("The tool doesn`t fit in the Working items!");
        if (App::isDebug())
            qDebug() << __FUNCTION__ << message;
        else
            QMessageBox::information(this, tr("Warning"), message);
        return;
    }

    file->setFileName(fileName_ + "_" + file->name());
    file->setSide(boardSide);
    if (fileId > -1) {
        exit(-123456);
        //        App::project().reload(fileId, file);
        //        editMode_ = false;
        //        fileId = -1;
    } else {
        App::project().addFile(file);
    }
}

void BaseForm::timerEvent(QTimerEvent* event) {
    if (event->timerId() == progressTimerId && progressDialog && creator_) {
        const auto [max, val] = creator_->getProgress();
        progressDialog->setMaximum(max);
        progressDialog->setValue(val);
        progressDialog->setLabelText(creator_->msg);
    }
}

Params* BaseForm::getNewGcp() {
    auto gcp = new GCode::Params;

    /*
    auto testFile = [&file, &skip, this](GraphicsItem* gi) -> bool {
        if (!file) {
            file = gi->file();
            boardSide = gi->file()->side();
        }
        if (file != gi->file()) {
            if (skip) {
                if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
                    return true;
            }
        }
        return {};
    };

    for (auto* item : App::graphicsView().selectedItems()) {
        auto gi = dynamic_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GiType::DataSolid:
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
            break;
        case GiType::DataPath:
            if (testFile(gi))
                return;
            wRawPaths.append(static_cast<GraphicsItem*>(item)->paths());
            break;
        case GiType::Drill:
            if (testFile(gi))
                return;
            wPaths.append(static_cast<GraphicsItem*>(item)->paths(1));
        default:
            break;
        }
        addUsedGi(gi);
    }
    */

    AbstractFile const* file = nullptr;
    bool skip {true};
    for (auto* gi : App::graphicsView().selectedItems<GraphicsItem>()) {
        switch (gi->type()) {
        case GiType::DataSolid:
            gcp->closedPaths.append(gi->paths());
            break;
        case GiType::DataPath: {
            auto paths = gi->paths();
            if (paths.front() == paths.back())
                gcp->closedPaths.append(paths);
            else
                gcp->openPaths.append(paths);
        } break;
            //            if (!file) {
            //                file = gi->file();
            //                boardSide = file->side();
            //            } else if (file != gi->file()) {
            //                if (skip) {
            //                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
            //                        return;
            //                }
            //            }
            //            if (gi->type() == GiType::DataSolid)
            //                gcp->closedPaths.append(gi->paths());
            //            else
            //                gcp->openPaths.append(gi->paths());
            //            break;
        case GiType::ShCircle:
        case GiType::ShRectangle:
        case GiType::ShText:
        case GiType::Drill:
            gcp->closedPaths.append(gi->paths());
            break;
        case GiType::ShPolyLine:
        case GiType::ShCirArc:
            gcp->openPaths.append(gi->paths());
            break;
        default:
            break;
        }
        addUsedGi(gi);
    }

    if (gcp->openPaths.empty() && gcp->closedPaths.empty()) {
        delete gcp;
        QMessageBox::warning(this, tr("Warning"), tr("No data for working..."));
        return nullptr;
    }

    return gcp;
}

void BaseForm::addUsedGi(GraphicsItem* gi) {
    if (gi->file()) {
        //        File const* file = gi->file();
        //        if (file->type() == FileType::Gerber_) {
        // #ifdef GBR_
        //            usedItems_[{file->id(), reinterpret_cast<const Gerber::File*>(file)->itemsType()}].push_back(gi->id());
        // #endif
        //        } else {
        //            usedItems_[{file->id(), -1}].push_back(gi->id());
        //        }
    }
}

void BaseForm::cancel() {
    if (creator_ == nullptr)
        return;
    creator_->continueCalc({});
    stopProgress();
}

void BaseForm::errorHandler(int) {
    if (creator_ == nullptr)
        return;

    if (creator_->items.empty())
        return;

    creator_->checkMillingFl = false;
    stopProgress();

    ctrWidget->setVisible(false);
    errWidget->setVisible(true);

    std::ranges::for_each(creator_->items, [](auto i) { App::graphicsView().addItem(i); });

    errTable->setModel(new ErrorModel(std::move(creator_->items), errTable));
    errTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    errTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    App::graphicsView().startUpdateTimer(32);
}

void BaseForm::errContinue() {
    if (creator_ == nullptr)
        return;
    qDebug(__FUNCTION__);
    App::graphicsView().stopUpdateTimer();

    delete errTable->model();

    ctrWidget->setVisible(true);
    errWidget->setVisible({});

    startProgress();
    creator_->continueCalc(true);
}

void BaseForm::errBreak() {
    if (creator_ == nullptr)
        return;
    qDebug(__FUNCTION__);
    App::graphicsView().stopUpdateTimer();

    delete errTable->model();

    ctrWidget->setVisible(true);
    errWidget->setVisible({});

    creator_->continueCalc(false);
}

void BaseForm::startProgress() {
    if (creator_ == nullptr)
        return;
    qDebug(__FUNCTION__);
    if (!fileCount)
        fileCount = 1;
    creator_->msg = fileName_;
    progressDialog->setLabelText(creator_->msg);
    progressTimerId = startTimer(100);
}

void BaseForm::stopProgress() {
    if (creator_ == nullptr)
        return;
    qDebug() << __FUNCTION__ << creator_->checkMillingFl;
    if (creator_->checkMillingFl)
        return;
    killTimer(progressTimerId);
    progressTimerId = 0;
    progressDialog->reset();
    progressDialog->hide();
}

} // namespace GCode

#include "moc_gc_baseform.cpp"
