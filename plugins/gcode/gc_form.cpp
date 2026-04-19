/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_form.h"

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

// #include <condition_variable>
// #include <mutex>
// #include <ranges>
// #include <sstream>

// static const int id1 = qRegisterMetaType<GCode::Params>("GCode::Params");
// static const int id2 = qRegisterMetaType<GCode::Params&>("GCode::Params&");
static const int id3 = qRegisterMetaType<GCode::File*>("GCode::File*");

namespace GCode {

inline QIcon errorIcon(const QPainterPath& path) {

    const QRectF rect = path.boundingRect();

    double scale = static_cast<double>(IconSize) / std::max(rect.width(), rect.height());

    double ky = rect.bottom() * scale;
    double kx = rect.left() * scale;
    if(rect.width() > rect.height())
        ky += (static_cast<double>(IconSize) - rect.height() * scale) / 2;
    else
        kx -= (static_cast<double>(IconSize) - rect.width() * scale) / 2;

    QPixmap pixmap{IconSize, IconSize};
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(App::settings().theme() > LightRed ? Qt::white : Qt::black);
    // painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    painter.drawPath(path);
    return pixmap;
}

class ErrorModel : public QAbstractTableModel {
    mvector<Gi::Error*> items;

public:
    ErrorModel(mvector<Gi::Error*>&& items, QObject* parent = nullptr)
        : QAbstractTableModel{parent}
        , items(std::move(items)) {
    }
    virtual ~ErrorModel() { qDeleteAll(items); }

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex&) const override { return static_cast<int>(items.size()); }
    int columnCount(const QModelIndex&) const override { return 2; }
    QVariant data(const QModelIndex& index, int role) const override {
        switch(role) {
        case Qt::DisplayRole:
            if(index.column() == 0) {
                auto pos{items[index.row()]->boundingRect().center()};
                return u"X = %1\nY = %2"_s.arg(pos.x()).arg(pos.y());
            }
            return items[index.row()]->area();
        case Qt::UserRole: return QVariant::fromValue(items[index.row()]);
        case Qt::DecorationRole:
            if(index.column() == 0)
                return errorIcon(items[index.row()]->shape());
            return {};
        case Qt::TextAlignmentRole:
            if(index.column() == 0)
                return Qt::AlignVCenter;
            return Qt::AlignCenter;
        default: break;
        }
        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if(section == 0)
                return QObject::tr("Position");
            else
                return QObject::tr("Area mm²");
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    void updateScene() {
        QRectF rect;
        for(auto item: items)
            if(item->isSelected())
                rect = rect.united(item->boundingRect());
        App::grView().fitInView(rect);
        // App::grView().zoomOut();
    }
};

class TableView : public QTableView {
    // Q_OBJECT
public:
    TableView(QWidget* parent)
        : QTableView{parent} {
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
        for(auto& var: selected.indexes())
            var.data(Qt::UserRole).value<Gi::Error*>()->setSelected(true);
        for(auto& var: deselected.indexes())
            var.data(Qt::UserRole).value<Gi::Error*>()->setSelected(false);
        static_cast<ErrorModel*>(model())->updateScene();
    }

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override {
        QTableView::showEvent(event);
        resizeRowsToContents();
    }
};

Form::Form(Plugin* plugin, Creator* tpc)
    : /*QWidget{this}
    ,*/
    plugin{plugin}
    , progressDialog(new QProgressDialog{this}) {

    auto vLayout = new QVBoxLayout{this};
    vLayout->addWidget(ctrWidget = new QWidget{this});
    vLayout->addWidget(errWidget = new QWidget{this});
    vLayout->setContentsMargins(0, 0, 0, 0);

    { // Creator
        content = new QWidget{ctrWidget};

        dsbxDepth = new DepthForm{plugin->gcName(), ctrWidget};
        dsbxDepth->setObjectName(u"dsbxDepth"_s);

        leName = new QLineEdit{ctrWidget};
        leName->setObjectName(u"leName"_s);

        pbClose = new QPushButton{tr("Close"), ctrWidget};
        pbClose->setIcon(QIcon::fromTheme(u"window-close"_s));
        pbClose->setObjectName(u"pbClose"_s);

        pbCreate = new QPushButton{tr("Create"), ctrWidget};
        pbCreate->setIcon(QIcon::fromTheme(u"document-export"_s));
        pbCreate->setObjectName(u"pbCreate"_s);
        connect(pbCreate, &QPushButton::clicked, this, &Form::computePaths);

        auto line = [this] {
            auto line = new QFrame{ctrWidget};
            line->setFrameShadow(QFrame::Plain);
            line->setFrameShape(QFrame::HLine);
            return line;
        };

        grid = new QGridLayout{ctrWidget};
        grid->setContentsMargins(6, 6, 6, 6);
        grid->setSpacing(6);

        int row{};
        constexpr int rowSpan{1}, columnSpan{2};
        // clang-format off
        grid->addWidget(dsbxDepth,                       row, 0, rowSpan, columnSpan); // row 0
        grid->addWidget(line(),                        ++row, 0, rowSpan, columnSpan); // row 1
        grid->addWidget(content,                       ++row, 0, rowSpan, columnSpan); // row 2
        grid->addWidget(line(),                        ++row, 0, rowSpan, columnSpan); // row 3
        grid->addWidget(new QLabel{tr("Name:"), this}, ++row, 0);                      // row 4
        grid->addWidget(leName,                          row, 1);                      // row 4
        grid->addWidget(pbCreate,                      ++row, 0, rowSpan, columnSpan); // row 5
        grid->addWidget(pbClose,                       ++row, 0, rowSpan, columnSpan); // row 6
        grid->addWidget(new QWidget{this},             ++row, 0, rowSpan, columnSpan); // row 7
        // clang-format on
        grid->setRowStretch(row, 1);
    }
    { // On Error
        auto grid = new QVBoxLayout{errWidget};
        grid->addWidget(errTable = new TableView{errWidget});
        grid->addWidget(errBtnBox = new QDialogButtonBox{errWidget});

        errBtnBox->setObjectName(u"errBtnBox"_s);
        errBtnBox->setOrientation(Qt::Horizontal);
        errBtnBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

        errBtnBox->button(QDialogButtonBox::Ok)->setText(tr("Continue"));
        errBtnBox->button(QDialogButtonBox::Cancel)->setText(tr("Break"));

        connect(errBtnBox->button(QDialogButtonBox::Ok),
            &QAbstractButton::clicked, this, &Form::errContinue);
        connect(errBtnBox->button(QDialogButtonBox::Cancel),
            &QAbstractButton::clicked, this, &Form::errBreak);
        errWidget->setVisible({});
    }

    progressDialog->setMinimumDuration(100);
    progressDialog->setModal(true);
    progressDialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);
    progressDialog->reset();
    connect(progressDialog, &QProgressDialog::canceled, this, &Form::cancel);
    setCreator(tpc);
}

Form::~Form() {
    if(errWidget->isVisible()) errBreak();
    ProgressCancel::cancel();
    if(runer.isRunning())
        // runer.terminate();
        // runer.quit();
        runer.wait();
    delete creator_;
    qDebug(__FUNCTION__);
}

void Form::setCreator(Creator* newCreator) {
    // qDebug() << __FUNCTION__ << creator_ << newCreator;
    ProgressCancel::cancel();
    if(runer.isRunning()) {
        // runer.terminate();
        // runer.quit();
        runer.wait();
    }
    ProgressCancel::reset();
    if(creator_ != newCreator && newCreator) {
        // qDebug(__FUNCTION__);
        delete creator_;
        creator_ = newCreator;
        creator_->moveToThread(&runer);
        // clang-format off
        // connect(&runer,  &QThread::finished,        creator_, &QObject::deleteLater                        );
        connect(creator_, &Creator::canceled,        this,     &Form::stopProgress                      );
        connect(creator_, &Creator::errorOccurred,   this,     &Form::errorHandler                      );
        connect(creator_, &Creator::fileReady,       this,     &Form::fileHandler                       );
         // connect(this,     &BaseForm::createToolpath, creator_, &Creator::createGc,      Qt::QueuedConnection);
        connect(this,     &Form::createToolpath, &runer,   &Runer::createGc,          Qt::QueuedConnection);
        connect(this,     &Form::createToolpath, this,     &Form::startProgress                     );
        // clang-format on
        // runer.start(QThread::LowPriority /*HighestPriority*/);
    } else if(creator_ && !newCreator) {
        creator_ = nullptr;
    }
}

void Form::fileHandler(File* file) {
    qDebug() << __FUNCTION__ << file;
    if(--fileCount == 0)
        cancel();

    if(file == nullptr) {
        auto message = tr("The tool doesn`t fit in the Working items!");
        if(App::isDebug())
            qDebug() << __FUNCTION__ << message;
        else
            QMessageBox::information(this, tr("Warning"), message);
        return;
    }

    file->setFileName(fileName_ + u"_"_s + file->name());
    file->setSide(boardSide);
    if(fileId > -1) {
        exit(-123456);
        // App::project().reload(fileId, file);
        // editMode_ = false;
        // fileId = -1;
    } else {
        App::project().addFile(file);
    }
}

void Form::timerEvent(QTimerEvent* event) {
    if(event->timerId() == progressTimerId && progressDialog && creator_) {
        const auto [max, val] = creator_->getProgress();
        progressDialog->setMaximum(max);
        progressDialog->setValue(val);
        progressDialog->setLabelText(creator_->msg);
    }
}

const Params& Form::getNewGcpWithGi() {
    usedItems_.clear();
    gcp = {}; // NOTE clear

    /*
    auto testFile = [&file, &skip, this](Gi::Item* gi) -> bool {
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

    for (auto* item : App::grView().selectedItems()) {
        auto gi = dynamic_cast<Gi::Item*>(item);
        switch (item->type()) {
        case Gi::Type::DataSolid:wPaths.append(static_cast<Gi::Item*>(item)->paths());break;
        case Gi::Type::DataPath:
            if (testFile(gi))
                return;
            wRawPaths.append(static_cast<Gi::Item*>(item)->paths());
            break;
        case Gi::Type::Drill:
            if (testFile(gi))
                return;
            wPaths.append(static_cast<Gi::Item*>(item)->paths(1));
        default: break;
        }
        addUsedGi(gi);
    }
    */

    // AbstractFile const* file = nullptr;
    // bool skip{true};

    int side{};
    for(auto* gi: App::grView().selectedItems<Gi::Item>()) {
        qDebug() << gi << gi->file();
        // switch(gi->type()) {
        // case Gi::Type::DataSolid:
        // gcp.closedPaths.append(gi->paths());
        // break;
        // case Gi::Type::DataPath: {
        // dbgPaths(gi->paths(), __FUNCTION__);
        for(auto&& curve: gi->curves()) {
            // qWarning() << curve;
            curve.isClosed() ? gcp.closedCurves.emplace_back(std::move(curve))
                             : gcp.openCurves.emplace_back(std::move(curve));
        }
        gi->side() == Side::Bottom ? --side : ++side;

        // } break;
        // // if (!file) {
        // // file = gi->file();
        // // boardSide = file->side();        // // } else if (file != gi->file()) {
        // // if (skip) {
        // // if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
        // // return;
        // // }
        // // }
        // // if (gi->type() == Gi::Type::DataSolid)
        // // gcp.closedPaths.append(gi->paths());
        // // else
        // // gcp.openPaths.append(gi->paths());
        // // break;
        // case Gi::Type::ShCircle:
        // case Gi::Type::ShRectangle:
        // case Gi::Type::ShText:
        // case Gi::Type::Drill:
        // gcp.closedPaths.append(gi->paths());
        // break;
        // case Gi::Type::ShPolyLine:
        // case Gi::Type::ShCirArc:
        // gcp.openPaths.append(gi->paths());
        // break;
        // default:  break;
        // }
        addUsedGi(gi);
    }
    qCritical() << "side" << side;
    gcp.params[Params::FileSide].setValue(side < 0 ? Side::Bottom : Side::Top);
    gcp.params[Params::GrItems].setValue(usedItems_);

    if(!gcp)
        QMessageBox::warning(this, tr("Warning"), tr("No data for working..."));

    return gcp;
}

void Form::addUsedGi(Gi::Item* gi) {
    if(auto file = gi->file(); file)
        usedItems_[{file->id(), file->itemsType()}].push_back(gi->id());
}

void Form::cancel() {
    if(creator_ == nullptr) return;

    creator_->continueCalc(false);
    ProgressCancel::cancel();
    if(runer.isRunning()) {
        // runer.quit();
        // runer.wait();
        runer.terminate();
        runer.wait();
    }
    // runer.start(QThread::LowPriority /*HighestPriority*/);
    stopProgress();
}

void Form::errorHandler(int) {
    if(creator_ == nullptr)
        return;

    if(creator_->items.empty())
        return;

    creator_->checkMillingFl = false;
    stopProgress();

    ctrWidget->setVisible(false);
    errWidget->setVisible(true);

    r::for_each(creator_->items, [](auto i) { App::grView().addItem(i); });

    errTable->setModel(new ErrorModel{std::move(creator_->items), errTable});
    errTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    errTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    App::grView().startUpdateTimer(32);
}

void Form::errContinue() {
    if(creator_ == nullptr)
        return;
    qDebug(__FUNCTION__);
    App::grView().stopUpdateTimer();

    delete errTable->model();

    ctrWidget->setVisible(true);
    errWidget->setVisible({});

    startProgress();
    creator_->continueCalc(true);
}

void Form::errBreak() {
    if(creator_ == nullptr)
        return;
    qDebug(__FUNCTION__);
    App::grView().stopUpdateTimer();

    delete errTable->model();

    ctrWidget->setVisible(true);
    errWidget->setVisible({});

    creator_->continueCalc(false);
}

void Form::startProgress() {
    if(creator_ == nullptr)
        return;
    qDebug(__FUNCTION__);
    if(!fileCount)
        fileCount = 1;
    creator_->msg = fileName_;
    progressDialog->setLabelText(creator_->msg);
    progressTimerId = startTimer(100);
}

void Form::stopProgress() {
    if(creator_ == nullptr)
        return;
    qDebug() << __FUNCTION__ << creator_->checkMillingFl;
    if(creator_->checkMillingFl)
        return;
    killTimer(progressTimerId);
    progressTimerId = 0;
    progressDialog->reset();
    progressDialog->hide();
}

} // namespace GCode

// #include "moc_gc_baseform.cpp"
