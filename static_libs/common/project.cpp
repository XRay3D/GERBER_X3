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
#include <project.h>

#include "abstract_file.h"
#include "abstract_fileplugin.h"
#include "ft_model.h"
#include "gc_file.h"
#include "gc_plugin.h"
#include "gi.h"
#include "graphicsview.h"
#include "reloadrequestdialog.h"
#include "shapepluginin.h"

#include <QElapsedTimer>
#include <QFileDialog>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>

const int isadfsdfg = qRegisterMetaType<AbstractFile*>("AbstractFile*");

Project::Project(QObject* parent)
    : QObject{parent}
    , watcher(this) {
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString& path) {
        if(!reloadPaths.emplace(path).second) return;
        // Проверка id ДО обращения к files_: contains() отдаёт -1 для файла не
        // из проекта, а files_[-1] у std::map вставляет пустой shared_ptr,
        // который прежний код тут же разыменовывал.
        if(contains(path) < 0 || !QFileInfo::exists(path)) {
            reloadPaths.erase(path);
            return;
        }
        // Не модальный вопрос на каждый файл, а строка в общем окне:
        // пересборка платы в САПР переписывает все слои разом.
        reloadDialog()->addRequest(path);
    });

    connect(this, &Project::addFileDbg, this, qOverload<GCode::File*>(&Project::addFile), Qt::QueuedConnection);

    App::setProject(this);
}

Project::~Project() {
    App::setProject(nullptr);
}

ReloadRequestDialog* Project::reloadDialog() {
    if(!reloadDialog_) {
        // Родитель -- главное окно, чтобы диалог не терялся за ним и уходил
        // вместе с приложением. Берём его через вид: MainWindow объявлен в
        // ggeasy, куда common не смотрит, а GraphicsView -- обычный QWidget,
        // и его window() и есть главное окно.
        auto* view = App::grViewPtr();
        reloadDialog_ = new ReloadRequestDialog{view ? view->window() : nullptr};
        connect(reloadDialog_, &ReloadRequestDialog::reloadRequested, this, [this](const QString& path) {
            const int id = contains(path);
            if(id < 0) { // файл успели закрыть, пока висел вопрос
                reloadPaths.erase(path);
                return;
            }
            // reloadPaths чистит Project::reload, когда разобранный файл придёт
            // обратно: до тех пор повторные срабатывания watcher'а по этому
            // пути гасятся, иначе САПР, дописывающий файл кусками, наплодил бы
            // строк.
            emit reloadFile(path, static_cast<int>(files_[id]->type()));
        });
        connect(reloadDialog_, &ReloadRequestDialog::skipRequested, this, [this](const QString& path) {
            // Забыли про этот файл -- о следующем его изменении спросим снова.
            reloadPaths.erase(path);
        });
    }
    return reloadDialog_;
}

Project::Header Project::header() const {
    Header h{
        .pinsPlaced = isPinsPlaced_,
        .tailing = tailing,
        .home = home_,
        .zero = zero_,
        .workRect = worckRect_,
        .safeZ = safeZ_,
        .boardThickness = boardThickness_,
        .copperThickness = copperThickness_,
        .clearence = clearence_,
        .plunge = plunge_,
        .glue = glue_,
        .viewRect = App::grView().getViewRect(),
    };
    std::ranges::copy(pins_, h.pins);
    std::ranges::copy(pinsUsed_, h.pinsUsed);
    return h;
}

void Project::applyHeader(const Header& h) {
    isPinsPlaced_ = h.pinsPlaced;
    tailing = h.tailing;
    home_ = h.home;
    zero_ = h.zero;
    worckRect_ = h.workRect;
    safeZ_ = h.safeZ;
    boardThickness_ = h.boardThickness;
    copperThickness_ = h.copperThickness;
    clearence_ = h.clearence;
    plunge_ = h.plunge;
    glue_ = h.glue;
    std::ranges::copy(h.pins, pins_);
    std::ranges::copy(h.pinsUsed, pinsUsed_);
}

bool Project::save(const QString& fileName) {
    QFile file{fileName};
    if(!file.open(QFile::WriteOnly)) {
        qDebug() << file.errorString();
        return false;
    }
    try {
        // Шапка — рефлексией по срезу полей проекта (см. Header в project.h).
        Serial::Writer sb;
        sb.start_object();
        sb.append_raw("\"ggeasy\":");
        Serial::write(sb, header());

        sb.append_raw(",\"files\":[");
        bool first = true;
        for(const auto& [id, filePtr]: files_) {
            const uint32_t type = filePtr->type();
            if(type == G_CODE || !type) continue; // база УП без операции не пишется (как раньше)
            std::string_view typeName;
            if(App::gCodePlugins().contains(type))
                typeName = App::gCodePlugin(type)->typeName();
            else if(App::filePlugins().contains(type))
                typeName = App::filePlugin(type)->typeName();
            if(typeName.empty()) {
                qWarning() << "file id" << id << "type" << type << "без typeName — пропущен";
                continue;
            }
            if(!first) sb.append_comma();
            first = false;
            sb.start_object();
            sb.append_raw("\"type\":");
            sb.escape_and_append_with_quotes(typeName);
            filePtr->serialize(sb); // поля конкретного класса, flatten
            sb.end_object();
        }

        sb.append_raw("],\"shapes\":[");
        first = true;
        for(const auto& [id, shape]: shapes_) {
            if(!shape) continue;
            std::string_view typeName;
            if(auto* plugin = App::shapePlugin(int(shape->type())); plugin)
                typeName = plugin->typeName();
            if(typeName.empty()) {
                qWarning() << "shape id" << id << "type" << shape->type() << "без typeName — пропущен";
                continue;
            }
            if(!first) sb.append_comma();
            first = false;
            sb.start_object();
            sb.append_raw("\"type\":");
            sb.escape_and_append_with_quotes(typeName);
            shape->serialize(sb);
            sb.end_object();
        }
        sb.append_raw("]");
        sb.end_object();

        std::string_view view;
        if(auto err = sb.view().get(view); err) {
            qCritical() << simdjson::error_message(err);
            return false;
        }
        if(file.write(view.data(), qint64(view.size())) != qint64(view.size())) {
            qDebug() << file.errorString();
            return false;
        }
        isModified_ = false;
        emit changed();
        return true;
    } catch(const std::exception& ex) {
        qDebug() << ex.what();
    } catch(...) {
        qDebug() << "Project::save failed";
    }
    return false;
}

bool Project::open(const QString& fileName) {
    QFile file{fileName};
    if(!file.open(QFile::ReadOnly)) {
        qDebug() << file.errorString();
        return false;
    }
    const QByteArray raw = file.readAll();
    try {
        Serial::Parsed parsed{
            std::string_view{raw.constData(), size_t(raw.size())}
        };
        simdjson::ondemand::object root;
        if(parsed.error || parsed.doc.get_object().get(root)) {
            const auto message = tr("The file is not a JSON project\n"
                                    "(the old binary format is not supported)\n"
                                    "or is corrupted.");
            qWarning() << fileName << message;
            if(!App::isDebug()) // headless/отладочный прогон не блокировать модальным окном
                QMessageBox::information(nullptr, tr("Project loading error"), message);
            return false;
        }

        QRectF sceneRect;
        {
            simdjson::ondemand::object headerObj;
            if(!root["ggeasy"].get_object().get(headerObj)) {
                Header h;
                Serial::readFields(headerObj, h);
                applyHeader(h);
                sceneRect = h.viewRect;
            }
        }

        {
            simdjson::ondemand::array arr;
            if(!root["files"].get_array().get(arr))
                for(auto elem: arr) {
                    simdjson::ondemand::value v;
                    if(elem.get(v)) continue;
                    std::string_view slice; // сырой текст элемента — им же кормится плагин
                    if(simdjson::to_json_string(v).get(slice)) continue;
                    Serial::Parsed peek{slice};
                    simdjson::ondemand::object obj;
                    if(peek.error || peek.doc.get_object().get(obj)) continue;
                    std::string_view typeName;
                    if(obj["type"].get_string().get(typeName)) continue;
                    const uint32_t type = md5::hash32(typeName);
                    AbstractFile* loaded{};
                    bool isFilePlugin{};
                    if(App::gCodePlugins().contains(type))
                        loaded = App::gCodePlugin(type)->loadFile(slice);
                    else if(App::filePlugins().contains(type))
                        loaded = App::filePlugin(type)->loadFile(slice), isFilePlugin = true;
                    else { // неизвестный тип: элемент просто пропускается
                        qWarning() << "unknown file type" << QLatin1StringView{typeName};
                        continue;
                    }
                    if(!loaded) {
                        qCritical() << "loadFile failed" << QLatin1StringView{typeName};
                        continue;
                    }
                    std::shared_ptr<AbstractFile> filePtr{loaded};
                    if(isFilePlugin && !watcher.files().contains(filePtr->name()))
                        watcher.addPath(filePtr->name());
                    filePtr->addToScene();
                    files_.emplace(filePtr->id(), std::move(filePtr));
                }
        }

        {
            simdjson::ondemand::array arr;
            if(!root["shapes"].get_array().get(arr))
                for(auto elem: arr) {
                    simdjson::ondemand::value v;
                    if(elem.get(v)) continue;
                    std::string_view slice;
                    if(simdjson::to_json_string(v).get(slice)) continue;
                    Serial::Parsed peek{slice};
                    simdjson::ondemand::object obj;
                    if(peek.error || peek.doc.get_object().get(obj)) continue;
                    std::string_view typeName;
                    if(obj["type"].get_string().get(typeName)) continue;
                    // У шейпов type() — маленький int (Gi::Type), не хэш имени:
                    // плагин ищется сканом реестра по typeName.
                    Shapes::Plugin* plugin{};
                    for(auto& [t, p]: App::shapePlugins())
                        if(p->typeName() == typeName) {
                            plugin = p;
                            break;
                        }
                    if(!plugin) { // неизвестный тип: элемент просто пропускается
                        qWarning() << "unknown shape type" << QLatin1StringView{typeName};
                        continue;
                    }
                    try {
                        auto* shape = plugin->createShape({std::nan(""), std::nan("")});
                        shape->deserialize(slice);
                        shapes_.emplace(shape->id(), shape);
                    } catch(const std::exception& ex) {
                        qCritical() << QLatin1StringView{typeName} << ex.what();
                    } catch(...) {
                        qCritical();
                    }
                }
        }

        for(const auto& [id, filePtr]: files_)
            App::fileModel().addFile(filePtr.get());

        // Иерархия УП не пишется в файл отдельно: она однозначно выводится из
        // имён программ. Многофайловая (несколько инструментов за один запуск)
        // складывается обратно в подпапку своего имени, одиночная остаётся
        // рядом со всеми -- ровно то же правило, что и при расчёте.
        {
            std::map<QString, std::vector<FileTree::Node*>> programs;
            for(const auto& [id, filePtr]: files_)
                if(auto* gc = dynamic_cast<GCode::File*>(filePtr.get()); gc && !gc->programName().isEmpty())
                    programs[gc->programName()].push_back(gc->node());
            for(const auto& [name, nodes]: programs)
                App::fileModel().groupProgram(name, nodes);
        }

        for(const auto& [id, shPtr]: shapes_)
            App::fileModel().addShape(shPtr);

        emit homePosChanged(home_);
        emit zeroPosChanged(zero_);
        emit pinsPosChanged(pins_);
        emit worckRectChanged(worckRect_);

        isModified_ = false;

        App::grView().setViewRectDeferred(sceneRect);

        return true;
    } catch(const QString& ex) {
        qDebug() << ex;
    } catch(const std::exception& ex) {
        qDebug() << ex.what();
    } catch(...) {
        qDebug() << errno;
    }
    return false;
}

void Project::close() {
    setWorckRect({});
    setStepsX(1);
    setStepsY(1);
    setSpaceX(0.0);
    setSpaceY(0.0);
    isPinsPlaced_ = false;
    isModified_ = false;
    for(auto& fl: pinsUsed_)
        fl = true;
    App::grView().zoom100();
    emit changed();
}

void Project::deleteFile(int32_t id) {
    std::lock_guard _{mutex};
    if(files_.contains(id)) {
        watcher.removePath(files_[id]->name());
        files_.erase(id);
        setChanged();
        // Данных стало меньше -- корневой прямоугольник BSP пора ужать.
        if(auto* view = App::grViewPtr()) view->scheduleSceneRectUpdate();
    } else
        qWarning() << u"Error id"_s << id << u"File not found"_s;
    isPinsPlaced_ = false;
}

void Project::deleteShape(int32_t id) {
    std::lock_guard _{mutex};
    try {
        if(shapes_.contains(id)) {
            shapes_.erase(id);
            setChanged();
        } else
            qWarning() << u"Error id"_s << id << u"AbstractShape not found"_s;
    } catch(const std::exception& ex) {
        qWarning() << ex.what();
    }
    isPinsPlaced_ = false;
}

int Project::addItem(Gi::Item* const item) {
    std::lock_guard _{mutex};
    if(!item)
        return -1;
    isPinsPlaced_ = false;
    item->id_ = items_.size() ? (--items_.end())->first + 1 : 0;
    item->setToolTip(QString::number(item->id_));
    item->setZValue(item->id_);

    App::grView().addItem(item);
    item->setColor(Qt::white);
    item->setVisible(true);

    items_.try_emplace(item->id_, item);
    App::fileModel().addItem(item);
    setChanged();
    return item->id_;
}

Gi::Item* Project::Item(int32_t id) {
    std::lock_guard _{mutex};
    return items_[id];
}

void Project::deleteItem(int32_t id) {
    std::lock_guard _{mutex};
    try {
        if(items_.contains(id)) {
            items_.erase(id);
            setChanged();
        } else
            qWarning() << u"Error id"_s << id << u"Gi::Item not found"_s;
    } catch(const std::exception& ex) {
        qWarning() << ex.what();
    }
    isPinsPlaced_ = false;
}

int Project::size() { return int(files_.size() + shapes_.size()); }

bool Project::isModified() { return isModified_; }

void Project::setModified(bool fl) { isModified_ = fl; }

QRectF Project::getBoundingRect() {
    std::lock_guard _{mutex};
    QRectF rect;
    for(const auto& [id, filePtr]: files_) {
        if(filePtr && filePtr->itemGroup()->isVisible()) {
            for(auto&& item: *filePtr->itemGroup()) {
                for(const Geo::Polyline& curve: item->curves()) {
                    if(rect.isNull())
                        rect = curve.boundingRect();
                    else
                        rect |= curve.boundingRect();
                }
            }
        }
    }
    return rect;
}

// QString Project::fileNames() {
// std::lock_guard _{mutex};
// QString fileNames;
// for (const auto& [id, sp] : files_) {
// AbstractFile* item = sp.get();
// if (sp && (item && (item->type() == FileType::Gerber_ || item->type() == FileType::Excellon_)))
// fileNames.append(item->name()).push_back(u'|');
// }
// return fileNames;
// }

int Project::contains(const QString& name) {
    // std::lock_guard _{mutex};
    // if(reloadPaths.contains(name))
    // return -1;
    for(const auto& [id, sp]: files_) {
        AbstractFile* item = sp.get();
        // if (sp && (item->type() == FileType::Gerber_ || item->type() == FileType::Excellon_ || item->type() == FileType::Dxf_))
        if(item && QFileInfo(item->name()).fileName() == QFileInfo(name).fileName())
            return item->id();
    }
    return -1;
}

bool Project::reload(int32_t id, AbstractFile* file) {
    reloadPaths.erase(file->name());
    if(files_.contains(id)) {
        file->initFrom(files_[id].get());
        files_[id].reset(file);
        // Плагины g-кода лежат в отдельной карте: App::filePlugin() для них
        // возвращает нуль, и разыменование роняло бы замену УП на месте.
        AbstractFilePlugin* plugin = App::filePlugin(file->type());
        if(!plugin) plugin = App::gCodePlugin(file->type());
        if(plugin) plugin->updateFileModel(file);
        setChanged();
        return true;
    }
    return false;
}

// Заменить УП на месте: узел дерева, id и сторона остаются прежними, меняется
// только содержимое. Отдельно от addFile, потому что там проверка на дубль имени
// отключена намеренно и попасть в ветку reload оттуда нельзя.
int Project::replaceFile(int32_t id, GCode::File* file) {
    std::lock_guard _{mutex};
    if(!file) return -1;
    isPinsPlaced_ = false;
    file->createGi();
    file->addToScene();
    file->setVisible(true);
    if(!reload(id, file)) return -1;
    return file->id();
}

std::vector<AbstractFile*> Project::files(uint32_t type) {
    std::lock_guard _{mutex};
    std::vector<AbstractFile*> rfiles;
    rfiles.reserve(files_.size());
    for(const auto& [id, sp]: files_)
        if(sp && sp->type() == type)
            rfiles.push_back(sp.get());
    rfiles.shrink_to_fit();
    return rfiles;
}

std::vector<AbstractFile*> Project::files(const std::vector<uint32_t>& types) {
    std::lock_guard _{mutex};
    std::vector<AbstractFile*> rfiles;
    rfiles.reserve(files_.size());
    for(auto type: types) {
        for(const auto& [id, sp]: files_)
            if(sp && sp->type() == type)
                rfiles.push_back(sp.get());
    }
    rfiles.shrink_to_fit();
    return rfiles;
}

Shapes::AbstractShape* Project::shape(int32_t id) {
    std::lock_guard _{mutex};
    return shapes_[id];
}

int Project::addFile(AbstractFile* file) {
    std::lock_guard _{mutex};
    if(!file) return -1;
    isPinsPlaced_ = false;
    file->createGi();
    file->addToScene();
    file->setVisible(true);
    const int32_t id = contains(file->name());
    if(id > -1 && files_[id]->type() == file->type()) {
        reload(id, file);
    } else if(file->id() == -1) {
        const int newId = files_.size() ? (--files_.end())->first + 1 : 0;
        file->setId(newId);
        files_.try_emplace(newId, file);
        App::fileModel().addFile(file);
        setChanged();
        watcher.addPath(file->name());
    }
    return file->id();
}

int Project::addFile(GCode::File* file) {
    std::lock_guard _{mutex};
    if(!file) return -1;
    isPinsPlaced_ = false;
    file->createGi();
    file->addToScene();
    file->setVisible(true);
    const int32_t id = -1; // contains(file->name());
    if(id > -1 && files_[id]->type() == file->type()) {
        reload(id, file);
    } else if(file->id() == -1) {
        const int newId = files_.size() ? (--files_.end())->first + 1 : 0;
        file->setId(newId);
        files_.try_emplace(newId, file);
        App::fileModel().addFile(file);
        setChanged();
        watcher.addPath(file->name());
    }
    return file->id();
}

int Project::addShape(Shapes::AbstractShape* const shape) {
    std::lock_guard _{mutex};
    if(!shape)
        return -1;
    isPinsPlaced_ = false;
    const int newId = shapes_.size() ? (--shapes_.end())->first + 1 : 0;
    shape->id_ = newId;
    shape->setToolTip(QString::number(newId));
    shape->setZValue(newId);
    shapes_.try_emplace(newId, shape); // NOTE destroy on filetree model
    App::fileModel().addShape(shape);
    setChanged();
    return newId;
}

int Project::makeShapeCircle(const QPointF& center, const QPointF& rad) {
    int type = Gi::Type::ShCircle;
    if(App::shapePlugins().contains(type)) {
        Shapes::AbstractShape* item = App::shapePlugin(type)->createShape(center);
        item->setPt(rad);
        addShape(item);
        item->setSelected(true);
        return 1;
    }
    return 0;
}

int Project::makeShapeRectangle(const QPointF& center, const QPointF& rect) {
    int type = Gi::Type::ShRectangle;
    if(App::shapePlugins().contains(type)) {
        Shapes::AbstractShape* item = App::shapePlugin(type)->createShape(center);
        item->setPt(rect);
        addShape(item);
        item->setSelected(true);
        return 1;
    }
    return 0;
}

bool Project::contains(AbstractFile* file) {
    for(const auto& [id, sp]: files_)
        if(sp.get() == file)
            return true;
    return false;
}

QString Project::name() { return fileName_; }

void Project::setName(const QString& name) {
    setUntitled(name.isEmpty());
    if(isUntitled_)
        fileName_ = QObject::tr("Untitled") + u".g2g"_s;
    else
        fileName_ = name;
}

void Project::setChanged() {
    isModified_ = true;
    changed();
}

bool Project::pinsPlacedMessage() {

    if(isPinsPlaced_ == false) {
        QMessageBox msgbx(QMessageBox::Information,
            {},
            QObject::tr("Board dimensions may have changed.\n"
                        "It is advisable to perform automatic placement of the pins\n"
                        "by selecting the necessary work items.\n\n"
                        "Continue saving?"),
            QMessageBox::Yes | QMessageBox::No, nullptr);
        {
            auto label(msgbx.findChild<QLabel*>());
            label->setPixmap(QIcon::fromTheme(u"snap-nodes-cusp"_s).pixmap(label->size()));
        }
        return msgbx.exec() == QMessageBox::No;
    }
    return false;
    /* Размеры платы могли измениться.
     * Выполните автоматическое размещение штифтов, выбрав необходимые рабочие элементы.
     * Продолжить сохранение?
     */
}

bool Project::isUntitled() { return isUntitled_; }

bool Project::isPinsPlaced() const { return isPinsPlaced_; }

void Project::setUntitled(bool value) {
    isUntitled_ = value;
    emit layoutFrameUpdate();
    setChanged();
}

double Project::spaceX() const { return tailing.spacingX; }
void Project::setSpaceX(double value) {
    tailing.spacingX = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

double Project::spaceY() const { return tailing.spacingY; }
void Project::setSpaceY(double value) {
    tailing.spacingY = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

uint Project::stepsX() const { return tailing.stepsX; }
void Project::setStepsX(uint value) {
    tailing.stepsX = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

uint Project::stepsY() const { return tailing.stepsY; }
void Project::setStepsY(uint value) {
    tailing.stepsY = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

QRectF Project::worckRect() const { return worckRect_; }
void Project::setWorckRect(const QRectF& worckRect) {
    worckRect_ = worckRect;
    isPinsPlaced_ = true;
    emit layoutFrameUpdate();
    setChanged();
}

QPointF Project::homePos() const { return home_; }
void Project::setHomePos(const QPointF& pos) {
    home_ = pos;
    setChanged();
}

QPointF Project::zeroPos() const { return zero_; }
void Project::setZeroPos(const QPointF& pos) {
    zero_ = pos;
    setChanged();
}

const QPointF* Project::pinsPos() const { return pins_; }
void Project::setPinsPos(const QPointF pos[4]) {
    pins_[0] = pos[0];
    pins_[1] = pos[1];
    pins_[2] = pos[2];
    pins_[3] = pos[3];
    setChanged();
}

bool Project::pinUsed(int idx) const { return pinsUsed_[idx]; }
void Project::setPinUsed(bool used, int idx) {
    pinsUsed_[idx] = used;
    setChanged();
}

double Project::safeZ() const { return safeZ_; }
void Project::setSafeZ(double safeZ) {
    safeZ_ = safeZ;
    setChanged();
}

double Project::boardThickness() const { return boardThickness_; }
void Project::setBoardThickness(double boardThickness) {
    boardThickness_ = boardThickness;
    setChanged();
}

double Project::copperThickness() const { return copperThickness_; }
void Project::setCopperThickness(double copperThickness) {
    copperThickness_ = copperThickness;
    setChanged();
}

double Project::clearence() const { return clearence_; }
void Project::setClearence(double clearence) {
    clearence_ = clearence;
    setChanged();
}

double Project::plunge() const { return plunge_; }
void Project::setPlunge(double plunge) {
    plunge_ = plunge;
    setChanged();
}

double Project::glue() const { return glue_; }
void Project::setGlue(double glue) {
    glue_ = glue;
    setChanged();
}

#include "moc_project.cpp"
