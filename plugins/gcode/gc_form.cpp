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
#include "ft_model.h"
#include "geo/boolean.h"
#include "gi_error.h"
#include "graphicsview.h"
#include "project.h"

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QIcon>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QTableView>
#include <QtWidgets>
#include <span>
#include <vector>

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
    painter.setBrush(App::settings().theme() == DarkTheme ? Qt::white : Qt::black);
    // painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    painter.drawPath(path);
    return pixmap;
}

class ErrorModel : public QAbstractTableModel {
    std::vector<Gi::Error*> items;

public:
    ErrorModel(std::vector<Gi::Error*>&& items, QObject* parent = nullptr)
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
    plugin{plugin} {

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
        // На время счёта эта же кнопка становится «Cancel»: место одно, а
        // состояния взаимоисключающие -- пока считаем, запускать нечего.
        connect(pbCreate, &QPushButton::clicked, this, [this] {
            computing_ ? cancel() : computePaths();
        });

        // Прогресс лежит ПОВЕРХ кнопки «Close»: он её ребёнок, и растянут по
        // ней своей компоновкой -- кнопка ездит вместе с доком, а полоса за
        // ней сама. В сетку формы полоса не входит вовсе, так что зазоры между
        // кнопками остаются те же, что были. На время счёта она закрывает
        // кнопку собой -- заодно и от нажатия.
        progressBar = new QProgressBar{pbClose};
        progressBar->setObjectName(u"progressBar"_s);
        progressBar->setTextVisible(true);
        progressBar->setVisible({});

        auto closeLayout = new QHBoxLayout{pbClose};
        closeLayout->setContentsMargins({});
        closeLayout->addWidget(progressBar);

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

    // Связка «нажали Create -> считаем» от Creator'а не зависит и живёт
    // ровно столько, сколько форма. В setCreator ей не место: drill меняет
    // Creator на ходу, и связка удваивалась бы с каждой сменой -- а с ней и
    // сам расчёт.
    connect(this, &Form::createToolpath, this, &Form::startCompute);

    setCreator(tpc);
}

Form::~Form() {
    // Поток может спать в isContinueCalc, ожидая ответа по найденным ошибкам:
    // не ответив, его не дождаться.
    if(errWidget->isVisible()) errBreak();
    stopWorker();
    delete creator_;
    qDebug(__FUNCTION__);
}

// Запуск расчёта: своё состояние потоку отдают ЦЕЛИКОМ (Params переезжают в
// лямбду), так что делить с ним после старта нечего.
void Form::startCompute(Params* newGcp) {
    if(creator_ == nullptr) return;
    // Прошлый прогон мог ещё доживать свои мгновения после отмены. Ждать его
    // здесь -- единственное место, где GUI замирает, и ровно на то время,
    // которое отменённому счёту нужно, чтобы упереться в ближайшую точку.
    stopWorker();

    startProgress();

    worker = std::jthread{[this, gcp = std::move(*newGcp)](std::stop_token token) mutable {
        creator_->createGc(std::move(gcp), std::move(token));
    }};
}

// Попросить остановиться и дождаться. Ровно это делает и деструктор jthread,
// но здесь оно нужно по имени -- перед перезапуском и перед сменой Creator'а.
void Form::stopWorker() { worker = {}; }

void Form::setCreator(Creator* newCreator) {
    // qDebug() << __FUNCTION__ << creator_ << newCreator;
    stopWorker();
    Progress::reset();
    if(creator_ != newCreator && newCreator) {
        delete creator_;
        creator_ = newCreator;
        // Creator остаётся в GUI-потоке: принадлежность нужна тому, кто
        // сигналы ПОЛУЧАЕТ, а ему их никто не шлёт. Свои же он посылает из
        // рабочего потока, и Qt доставит их сюда очередью -- как раз потому,
        // что живёт эта форма здесь.
        connect(creator_, &Creator::errorOccurred, this, &Form::errorHandler);
        connect(creator_, &Creator::fileReady, this, &Form::fileHandler);
    } else if(creator_ && !newCreator) {
        creator_ = nullptr;
    }
}

void Form::fileHandler(File* file) {
    qDebug() << __FUNCTION__ << file;

    // Прогон отменён: расчёт узнаёт об этом на ближайшей своей точке отмены и
    // до тех пор шлёт файлы как ни в чём не бывало. Они уже не наши -- ни в
    // проект, ни в дерево им не надо, а удалить их, кроме нас, некому.
    if(canceled_) {
        delete file;
        return;
    }

    if(--fileCount == 0)
        stopProgress();

    if(file == nullptr) {
        auto message = tr("The tool doesn`t fit in the Working items!");
        if(App::isDebug())
            qDebug() << __FUNCTION__ << message;
        else
            QMessageBox::information(this, tr("Warning"), message);
        return;
    }

    file->setProgramName(fileName_);
    file->setVisibility(visibility_);
    file->setFileName(fileName_ + u"_"_s + file->name());
    // Сторону задаёт файл: её вычислило голосование по выделенным элементам в
    // getNewGcpWithGi и применил конструктор File, а при замене на месте её
    // перенесёт initFrom со старого файла. Форма сторону только отражает.
    boardSide = file->side();

    if(!replacedIds_.empty()) {
        App::project().replaceFile(replacedIds_.front(), file);
        replacedIds_.erase(replacedIds_.begin());
    } else
        App::project().addFile(file);

    // Свежий прогон (fileId ещё не назначен): запоминаем id ПЕРВОГО пришедшего
    // файла, не последнего. files_ в Project -- std::map<int,...>, id раздаются
    // по возрастанию в порядке добавления (Project::addFile), а подчистка
    // "лишних" ниже съедает replacedIds_ front-first в том же порядке -- значит
    // именно САМЫЙ ПЕРВЫЙ id переживёт любое будущее сокращение числа файлов
    // программы (PocketOffset: было 3 инструмента, стало 2), а не последний.
    if(fileId < 0 && autoEnterEditMode()) fileId = file->id();

    // Прогон окончен (fileCount обнулился выше). Прежних файлов под этим именем
    // могло быть больше, чем новых -- скажем, у PocketOffset убрали инструмент.
    // Лишние убираем вместе с их строками в дереве: файл живёт ровно столько,
    // сколько его узел.
    if(fileCount == 0) {
        for(int32_t id: replacedIds_)
            if(auto* stale = App::project().file(id); stale)
                App::fileModel().removeNode(stale->node());
        replacedIds_.clear();

        // Многофайловая УП уезжает в подпапку своего имени, одиночная остаётся
        // рядом со всеми. Порядок важен: лишние узлы уже убраны, так что счёт
        // здесь -- окончательный.
        std::vector<FileTree::Node*> nodes;
        for(int32_t id: programIds(fileName_))
            if(auto* f = App::project().file(id); f) nodes.push_back(f->node());
        App::fileModel().groupProgram(fileName_, nodes);
        // Форма остаётся в режиме правки: следующий клик Create в этой же
        // сессии показа формы под тем же именем тихо перепишет ту же УП (см.
        // ветку editMode_ && origName == target в resolveFileName), а не
        // спросит. На явное открытие существующей УП через дерево это не
        // влияет -- там fileId уже был >-1 до начала прогона.
        if(fileId > -1) setEditMode(true);
    }
}

void Form::editFile(File* file) {
    setEditMode(true);
    fileId = file->id();
    gcp = file->params();
    // Сторону задаёт файл, форма её только отражает.
    boardSide = file->side();

    if(auto it = gcp.params.find(Params::Depth); it != gcp.params.end())
        dsbxDepth->setValue(it->second.toDouble());

    // Имя ставится последним: у плагинов rb_clicked() дёргает updateName(), и
    // тот перебил бы его именем по умолчанию для выбранной стороны.
    fileName_ = file->programName();
    leName->setText(fileName_);

    restoreContext(file);
}

void Form::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // Форма -- поле плагина и переживает закрытие дока вместе со всем своим
    // состоянием. Открытая с панели, она должна создавать новую УП, а не молча
    // переписывать ту, что правили в прошлый раз: режим правки включает только
    // editFile(), а он зовётся уже ПОСЛЕ показа формы в доке.
    setEditMode(false);
    fileId = -1;
    replacedIds_.clear();
}

void Form::setEditMode(bool on) {
    editMode_ = on;
    // Пока считаем, на кнопке «Cancel»: надпись вернёт setComputing.
    if(!computing_) pbCreate->setText(on ? tr("Update") : tr("Create"));
}

void Form::restoreContext(File* file) {
    // Видимость -- по снимку. Файлы, которых на момент расчёта не было (включая
    // саму эту УП), в снимке отсутствуют и не трогаются.
    for(auto&& [id, visible]: file->visibility())
        if(auto* f = App::project().file(id); f && f->isVisible() != visible) {
            f->setVisible(visible);
            App::fileModel().updateNode(f->node()); // галочка в дереве
        }

    App::grView().scene()->clearSelection();

    auto it = gcp.params.find(Params::GrItems);
    if(it == gcp.params.end()) return;
    for(auto&& [key, ids]: it->second.value<UsedItems>()) {
        if(key.size() < 2) continue; // ключ -- пара {id файла, тип элементов}
        if(auto* f = App::project().file(key[0]); f)
            f->itemGroup(key[1])->setSelected(ids);
    }
}

void Form::timerEvent(QTimerEvent* event) {
    if(event->timerId() == progressTimerId && creator_) {
        const auto [max, val] = creator_->getProgress();
        // Нулевой максимум для QProgressBar -- «шагов не знаю»: полоса бежит
        // сама. Врать долей выполненного там, где её никто не считал, хуже.
        progressBar->setMaximum(max);
        progressBar->setValue(val);
        progressBar->setFormat(max ? creator_->message() + u" %p%"_s : creator_->message());
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
    // Выделенное делится по тому, ОТКУДА известна вложенность:
    //
    //   * у заливки (DataSolid) регион точный -- дырки каждого полигона
    //     известны, и такие куски объединяются готовыми полигонами. Плоский
    //     список контуров здесь терял тело, лежащее внутри чужой дырки: на
    //     реальной плате это срезало 52 тела до 8 и раздувало площадь на 18%;
    //
    //   * у остальных есть только контуры, и вложенность МЕЖДУ элементами
    //     разбирается even-odd по их взаимному расположению -- всем выделением
    //     разом. Пообъектно (каждый контур своим телом, потом union) рамка
    //     проглатывала выделенные вместе с ней отверстия внутри неё.
    std::vector<Geo::Polygon> closedParts;
    Geo::Polylines closedContours;
    for(auto* gi: App::grView().selectedItems<Gi::Item>()) {
        qDebug() << gi << gi->file();
        // switch(gi->type()) {
        // case Gi::Type::DataSolid:
        // gcp.closedPaths.append(gi->curves());
        // break;
        // case Gi::Type::DataPath: {
        // dbgPaths(gi->curves(), __FUNCTION__);
        const bool exact = gi->type() == Gi::Type::DataSolid;
        // Разомкнутое -- отдельно: площади у линии нет, в регион ей не попасть.
        for(auto&& curve: gi->curves())
            if(!curve.isClosed())
                gcp.openCurves.emplace_back(std::move(curve));
            else if(!exact)
                closedContours.emplace_back(std::move(curve));
        if(exact) closedParts.append_range(gi->region().all());
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
        // // gcp.closedPaths.append(gi->curves());
        // // else
        // // gcp.openPaths.append(gi->curves());
        // // break;
        // case Gi::Type::ShCircle:
        // case Gi::Type::ShRectangle:
        // case Gi::Type::ShText:
        // case Gi::Type::Drill:
        // gcp.closedPaths.append(gi->curves());
        // break;
        // case Gi::Type::ShPolyLine:
        // case Gi::Type::ShCirArc:
        // gcp.openPaths.append(gi->curves());
        // break;
        // default:  break;
        // }
        addUsedGi(gi);
    }
    gcp.closedCurves = Geo::evenOdd(closedContours);
    if(!closedParts.empty())
        gcp.closedCurves |= Geo::Polygons{std::span<const Geo::Polygon>{closedParts}};

    qCritical() << "side" << side;
    gcp.params[Params::FileSide].setValue(side < 0 ? Side::Bottom : Side::Top);
    gcp.params[Params::GrItems].setValue(usedItems_);

    if(!gcp) {
        QMessageBox::warning(this, tr("Warning"), tr("No data for working..."));
        return gcp;
    }

    // Видимость снимается со ВСЕГО проекта, а не только с использованных файлов:
    // вернуть надо ту картину, что была перед нажатием Create, вместе с тем, что
    // пользователь намеренно погасил.
    visibility_.clear();
    for(auto* file: App::project().files())
        visibility_.emplace(file->id(), file->isVisible());

    // Отказ выражается пустыми параметрами: все плагины уже проверяют результат
    // getNewGcpWithGi() на ложность первой же строкой computePaths().
    if(!resolveFileName()) gcp = {};

    return gcp;
}

File* Form::findProgram(const QString& programName, int32_t exceptId) const {
    for(auto* file: App::project().files<File>())
        if(file->id() != exceptId && file->programName() == programName)
            return file;
    return nullptr;
}

std::vector<int32_t> Form::programIds(const QString& programName) const {
    std::vector<int32_t> ids;
    for(auto* file: App::project().files<File>())
        if(file->programName() == programName)
            ids.push_back(file->id());
    return ids;
}

QString Form::uniqueProgramName(QString name) const {
    static const QRegularExpression tail{uR"(\s*\((\d+)\)$)"_s};
    int number = 2;
    // Хвостовой номер наращивается, а не вкладывается: из "Profile (2)" выходит
    // "Profile (3)", а не "Profile (2) (2)".
    if(auto match = tail.match(name); match.hasMatch()) {
        number = match.captured(1).toInt() + 1;
        name.truncate(match.capturedStart());
    }
    QString candidate;
    do candidate = u"%1 (%2)"_s.arg(name).arg(number++);
    while(findProgram(candidate, fileId));
    return candidate;
}

bool Form::resolveFileName() {
    // Три кнопки по образцу MainWindow::maybeSave(): две осмысленных и отмена.
    auto ask = [this](const QString& text, const QString& first, const QString& second) {
        QMessageBox box{QMessageBox::Question, tr("Toolpath name"), text, QMessageBox::Cancel, this};
        auto* firstButton = box.addButton(first, QMessageBox::AcceptRole);
        auto* secondButton = box.addButton(second, QMessageBox::ActionRole);
        box.setDefaultButton(firstButton);
        box.exec();
        if(box.clickedButton() == firstButton) return 0;
        if(box.clickedButton() == secondButton) return 1;
        return -1;
    };

    QString target = fileName_;
    replacedIds_.clear();

    if(editMode_ && fileId > -1) {
        auto* orig = dynamic_cast<File*>(App::project().file(fileId));
        const QString origName = orig ? orig->programName() : QString{};
        // Правка под тем же именем перезаписывает файл молча -- это и есть
        // обычный повторный расчёт уже существующей УП.
        if(orig && origName == target) {
            replacedIds_ = programIds(origName);
            return true;
        }
        switch(ask(tr("Toolpath \"%1\" is open for editing, but the name has been changed to \"%2\".")
                       .arg(origName, target),
            tr("Rename"), tr("Save as New"))) {
        case 0: // переименовать: заменяем те же файлы, но под новым именем
            replacedIds_ = programIds(origName);
            break;
        case 1: // сохранить как новую: дальше пойдём общим путём создания
            setEditMode(false);
            fileId = -1;
            break;
        default: return false;
        }
    }

    // exceptId исключает саму правящуюся УП: её собственное имя коллизией не
    // считается -- на этой ветке оно как раз меняется на target.
    while(auto* dup = findProgram(target, fileId)) {
        switch(ask(tr("Toolpath \"%1\" already exists in the project.").arg(target),
            tr("Replace"), tr("Add Number"))) {
        case 0: // заменить: под этим именем может лежать не один файл
            replacedIds_ = programIds(target);
            fileId = dup->id();
            setEditMode(true);
            break;
        case 1: // uniqueProgramName сам крутится до свободного имени
            target = uniqueProgramName(target);
            continue;
        default: return false;
        }
        break;
    }

    fileName_ = target;
    if(leName->text() != target) leName->setText(target);
    return true;
}

void Form::addUsedGi(Gi::Item* gi) {
    if(auto file = gi->file(); file)
        usedItems_[{file->id(), file->itemsType()}].push_back(gi->id());
}

// Отмена: кнопка «Cancel» на месте «Create» либо «Break» в таблице ошибок.
// Ждать здесь поток нельзя -- отменяют как раз затем, чтобы окно ожило, а
// расчёт свернётся сам, упёршись в ближайшую единицу работы Geo. Прежде тут
// стоял terminate() -- убийство потока посреди CGAL, то есть повреждённая
// куча и утечки.
void Form::cancel() {
    if(creator_ == nullptr) return;

    canceled_ = true;
    // Счёт этого прогона больше никого не интересует, и файлов от него не
    // ждут: следующий запуск назначит своё число сам.
    fileCount = 0;
    worker.request_stop();    // Geo свернёт счёт на ближайшей единице работы
    creator_->continueCalc(); // если поток спит на ошибках -- разбудить
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

    // Таймер анимации больше не общий: Gi::Error регистрируется в реестре вида
    // сам, когда его выделяют. Прежние startUpdateTimer/stopUpdateTimer здесь
    // ещё и убивали таймер, заведённый конструктором вида, -- насовсем.
}

void Form::errContinue() {
    if(creator_ == nullptr)
        return;
    qDebug(__FUNCTION__);

    delete errTable->model();

    ctrWidget->setVisible(true);
    errWidget->setVisible({});

    startProgress();
    creator_->continueCalc();
}

void Form::errBreak() {
    if(creator_ == nullptr)
        return;
    qDebug(__FUNCTION__);

    delete errTable->model();

    ctrWidget->setVisible(true);
    errWidget->setVisible({});

    // «Break» -- та же отмена: расчётный поток ждёт ответа, и ответ ему
    // «дальше не считаем».
    cancel();
}

void Form::startProgress() {
    if(creator_ == nullptr)
        return;
    qDebug(__FUNCTION__);
    canceled_ = false;
    if(!fileCount)
        fileCount = 1;
    creator_->setMsg(fileName_);
    if(progressTimerId) killTimer(progressTimerId); // перезапуск поверх прошлого
    progressBar->setFormat(fileName_);
    progressBar->setMaximum(0);
    progressBar->setValue(0);
    setComputing(true);
    progressTimerId = startTimer(100);
}

void Form::stopProgress() {
    if(creator_ == nullptr)
        return;
    qDebug() << __FUNCTION__ << creator_->checkMillingFl;
    if(creator_->checkMillingFl)
        return;
    if(progressTimerId) killTimer(progressTimerId);
    progressTimerId = 0;
    setComputing(false);
}

// Форма на время счёта: кнопка запуска становится отменой, а прогресс занимает
// место «Close». Закрывать форму, пока считает её же Creator, нельзя -- на
// выходе ~Form дождётся потока, а до тех пор окно всё равно стоит.
void Form::setComputing(bool on) {
    if(computing_ == on) return;
    computing_ = on;

    if(on) {
        createEnabled_ = pbCreate->isEnabled();
        // Плагины гасят кнопку по своим условиям (drill, thread): отменять
        // счёт это мешать не должно.
        pbCreate->setEnabled(true);
    } else
        pbCreate->setEnabled(createEnabled_);

    if(on)
        pbCreate->setText(tr("Cancel"));
    else
        pbCreate->setText(editMode_ ? tr("Update") : tr("Create"));
    pbCreate->setIcon(QIcon::fromTheme(on ? u"process-stop"_s : u"document-export"_s));

    progressBar->setVisible(on);
}

} // namespace GCode

// #include "moc_gc_baseform.cpp"
