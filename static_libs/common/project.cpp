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
#include <project.h>

#include "abstract_file.h"
#include "abstract_fileplugin.h"
#include "ft_model.h"
#include "gc_file.h"
#include "gc_plugin.h"
#include "gi.h"
#include "graphicsview.h"
#include "shapepluginin.h"

#include <QElapsedTimer>
#include <QFileDialog>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>

const int isadfsdfg = qRegisterMetaType<AbstractFile*>("AbstractFile*");

QDataStream& operator<<(QDataStream& stream, const std::shared_ptr<AbstractFile>& file) {
    if(file->type() == G_CODE)
        return stream;
    if(file->type()) {
        uint32_t type = file->type();
        stream << type;
        if(App::gCodePlugins().contains(type))
            stream << App::gCodePlugin(type)->info()["Name"].toString();
        else if(App::filePlugins().contains(type))
            stream << App::filePlugin(type)->info()["Name"].toString();
        else
            stream << QString("QString");
        stream << *file;
    }
    return stream;
}

QDataStream& operator>>(QDataStream& stream, std::shared_ptr<AbstractFile>& file) {
    uint32_t type;
    QString loadErrorMessage;
    stream >> type;
    stream >> loadErrorMessage;
    qDebug() << __FUNCTION__ << "type" << type << loadErrorMessage;
    if(App::gCodePlugins().contains(type))
        file.reset(App::gCodePlugin(type)->loadFile(stream));
    else if(App::filePlugins().contains(type)) {
        file.reset(App::filePlugin(type)->loadFile(stream));
        if(!App::project().watcher.files().contains(file->name()))
            App::project().watcher.addPath(file->name());
    } else {
        QByteArray data;
        return stream >> data;
    }

    file->addToScene();
    return stream;
}

QDataStream& operator<<(QDataStream& stream, Shapes::AbstractShape* shape) {
    stream << shape->type();
    stream << shape->name();
    stream << *shape;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Shapes::AbstractShape*& shape) {
    uint32_t type;
    QString loadErrorMessage;
    stream >> type;
    stream >> loadErrorMessage;
    if(App::shapePlugins().contains(type)) {
        shape = App::shapePlugin(type)->createShape();
        stream >> *shape;
        App::grView().addItem(shape);
    } else {
        QByteArray data;
        stream >> data;
        qDebug() << type << loadErrorMessage << data;
    }
    return stream;
}

QDataStream& operator<<(QDataStream& stream, Gi::Item* shape) {
    stream << uint32_t(shape->type());
    stream << shape->paths();
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Gi::Item*& shape) {
    uint32_t type;
    Paths paths;
    stream >> type;
    stream >> paths;
    //    if(App::shapePlugins().contains(type)) {
    //        shape = App::shapePlugin(type)->createShape();
    //        stream >> *shape;
    //        App::grView().addItem(shape);
    //    } else {
    //        QByteArray data;
    //        stream >> data;
    //        qDebug() << type << loadErrorMessage << data;
    //    }
    return stream;
}

Project::Project(QObject* parent)
    : QObject(parent)
    , watcher(this) {
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString& path) {
        const int32_t id = files_[contains(path)]->id();
        if(id > -1
            && QFileInfo::exists(path)
            && QMessageBox::question(nullptr, "", tr("External file \"%1\" has changed.\nReload it into the project?").arg(QFileInfo(path).fileName()),
                   QMessageBox::Ok, QMessageBox::Cancel)
                == QMessageBox::Ok) {
            reloadFile_ = true;
            emit reloadFile(path, static_cast<int>(files_[id]->type()));
        }
    });

    connect(this, &Project::addFileDbg, this, qOverload<GCode::File*>(&Project::addFile), Qt::QueuedConnection);

    App::setProject(this);
}

Project::~Project() {
    App::setProject(nullptr);
}

bool Project::save(const QString& fileName) {
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly)) {
        qDebug() << file.errorString();
        return false;
    }
    QDataStream out(&file);
    try {
        out << (ver_ = CurrentVer);
        Block(out).write(
            isPinsPlaced_,
            tailing,
            home_,
            zero_,
            pins_,
            pinsUsed_,
            worckRect_,
            safeZ_,
            boardThickness_,
            copperThickness_,
            clearence_,
            plunge_,
            glue_,
            App::grView().getViewRect());
        out << files_;
        out << shapes_;
        out << items_;
        isModified_ = false;
        emit changed();
        return true;
    } catch(...) {
        qDebug() << out.status();
    }
    return false;
}

bool Project::open(const QString& fileName) {
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly)) {
        qDebug() << file.errorString();
        return false;
    }
    QDataStream in(&file);
    try {
        in >> ver_;
        if(ver_ < CurrentVer) {
            auto message = tr("Unable to load project version %1 in\n"
                              "the current version(%3) of the program.\n"
                              "Use version %2.");
            if(App::isDebug()) {
                qWarning() << message.arg(ver_).arg("???", "VERSION_STR");
                return false;
            }
            switch(ver_) {
            case ProVer_1:
            case ProVer_2:
            case ProVer_3:
            case ProVer_4:
            case ProVer_5:
            case ProVer_6:
            case ProVer_7:
                QMessageBox::information(nullptr, tr("Project loading error"), message.arg(ver_).arg("???", "VERSION_STR"));
                break;
            }
            return false;
        }
        QRectF sceneRect;
        Block(in).read(
            isPinsPlaced_,
            tailing,
            home_,
            zero_,
            pins_,
            pinsUsed_,
            worckRect_,
            safeZ_,
            boardThickness_,
            copperThickness_,
            clearence_,
            plunge_,
            glue_,
            sceneRect);
        in >> files_;
        in >> shapes_;
        // in >> items_;
        for(const auto& [id, filePtr]: files_)
            App::fileModel().addFile(filePtr.get());
        for(const auto& [id, shPtr]: shapes_)
            App::fileModel().addShape(shPtr);
        for(const auto& [id, itemPtr]: items_)
            App::fileModel().addItem(itemPtr);

        emit homePosChanged(home_);
        emit zeroPosChanged(zero_);
        emit pinsPosChanged(pins_);
        emit worckRectChanged(worckRect_);

        isModified_ = false;

        App::grView().fitInView(sceneRect, false);

        return true;
    } catch(const QString& ex) {
        qDebug() << ex;
    } catch(const std::exception& ex) {
        qDebug() << ex.what();
    } catch(...) {
        qDebug() << in.status();
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
    QMutexLocker locker(&mutex);
    if(files_.contains(id)) {
        watcher.removePath(files_[id]->name());
        files_.erase(id);
        setChanged();
    } else
        qWarning() << "Error id" << id << "File not found";
    isPinsPlaced_ = false;
}

void Project::deleteShape(int32_t id) {
    QMutexLocker locker(&mutex);
    try {
        if(shapes_.contains(id)) {
            shapes_.erase(id);
            setChanged();
        } else
            qWarning() << "Error id" << id << "AbstractShape not found";
    } catch(const std::exception& ex) {
        qWarning() << ex.what();
    }
    isPinsPlaced_ = false;
}

int Project::addItem(Gi::Item* const item) {
    QMutexLocker locker(&mutex);
    if(!item)
        return -1;
    isPinsPlaced_ = false;
    item->id_ = items_.size() ? (--items_.end())->first + 1 : 0;
    item->setToolTip(QString::number(item->id_));
    item->setZValue(item->id_);

    App::grView().addItem(item);
    item->setColor(Qt::white);
    item->setVisible(true);

    items_.emplace(item->id_, item);
    App::fileModel().addItem(item);
    setChanged();
    return item->id_;
}

Gi::Item* Project::Item(int32_t id) {
    QMutexLocker locker(&mutex);
    return items_[id];
}

void Project::deleteItem(int32_t id) {
    QMutexLocker locker(&mutex);
    try {
        if(items_.contains(id)) {
            items_.erase(id);
            setChanged();
        } else
            qWarning() << "Error id" << id << "Gi::Item not found";
    } catch(const std::exception& ex) {
        qWarning() << ex.what();
    }
    isPinsPlaced_ = false;
}

int Project::size() { return int(files_.size() + shapes_.size()); }

bool Project::isModified() { return isModified_; }

void Project::setModified(bool fl) { isModified_ = fl; }

QRectF Project::getBoundingRect() {
    QMutexLocker locker(&mutex);
    Point topLeft(std::numeric_limits</*Point::Type*/ int32_t>::max(), std::numeric_limits</*Point::Type*/ int32_t>::max());
    Point botRight(std::numeric_limits</*Point::Type*/ int32_t>::min(), std::numeric_limits</*Point::Type*/ int32_t>::min());
    for(const auto& [id, filePtr]: files_) {
        if(filePtr && filePtr->itemGroup()->isVisible()) {
            for(const Gi::Item* const item: *filePtr->itemGroup()) {
                for(const Path& path: item->paths()) {
                    for(const Point& pt: path) {
                        topLeft.x = std::min(pt.x, topLeft.x);
                        topLeft.y = std::min(pt.y, topLeft.y);
                        botRight.x = std::max(pt.x, botRight.x);
                        botRight.y = std::max(pt.y, botRight.y);
                    }
                }
            }
        }
    }
    return QRectF{~topLeft, ~botRight};
}

// QString Project::fileNames() {
//     QMutexLocker locker(&mutex);
//     QString fileNames;
//     for (const auto& [id, sp] : files_) {
//         AbstractFile* item = sp.get();
//         if (sp && (item && (item->type() == FileType::Gerber_ || item->type() == FileType::Excellon_)))
//             fileNames.append(item->name()).push_back('|');
//     }
//     return fileNames;
// }

int Project::contains(const QString& name) {
    // QMutexLocker locker(&mutex);
    if(reloadFile_)
        return -1;
    for(const auto& [id, sp]: files_) {
        AbstractFile* item = sp.get();
        //        if (sp && (item->type() == FileType::Gerber_ || item->type() == FileType::Excellon_ || item->type() == FileType::Dxf_))
        if(QFileInfo(item->name()).fileName() == QFileInfo(name).fileName())
            return item->id();
    }
    return -1;
}

bool Project::reload(int32_t id, AbstractFile* file) {
    if(files_.contains(id)) {
        file->initFrom(files_[id].get());
        files_[id].reset(file);
        App::filePlugin(file->type())->updateFileModel(file);
        setChanged();
        return true;
    }
    return false;
}

mvector<AbstractFile*> Project::files(int type) {
    QMutexLocker locker(&mutex);
    mvector<AbstractFile*> rfiles;
    rfiles.reserve(files_.size());
    for(const auto& [id, sp]: files_)
        if(sp && sp->type() == type)
            rfiles.push_back(sp.get());
    rfiles.shrink_to_fit();
    return rfiles;
}

mvector<AbstractFile*> Project::files(const mvector<int> types) {
    QMutexLocker locker(&mutex);
    mvector<AbstractFile*> rfiles;
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
    QMutexLocker locker(&mutex);
    return shapes_[id];
}

int Project::addFile(AbstractFile* file) {
    QMutexLocker locker(&mutex);
    if(!file)
        return -1;
    isPinsPlaced_ = false;
    reloadFile_ = false;
    file->createGi();
    file->addToScene();
    file->setVisible(true);
    const int32_t id = contains(file->name());
    if(id > -1 && files_[id]->type() == file->type()) {
        reload(id, file);
    } else if(file->id() == -1) {
        const int newId = files_.size() ? (--files_.end())->first + 1 : 0;
        file->setId(newId);
        files_.emplace(newId, file);
        App::fileModel().addFile(file);
        setChanged();
        watcher.addPath(file->name());
    }
    return file->id();
}

int Project::addFile(GCode::File* file) {
    QMutexLocker locker(&mutex);
    if(!file)
        return -1;
    isPinsPlaced_ = false;
    reloadFile_ = false;
    file->createGi();
    file->addToScene();
    file->setVisible(true);
    const int32_t id = -1; // contains(file->name());
    if(id > -1 && files_[id]->type() == file->type()) {
        reload(id, file);
    } else if(file->id() == -1) {
        const int newId = files_.size() ? (--files_.end())->first + 1 : 0;
        file->setId(newId);
        files_.emplace(newId, file);
        App::fileModel().addFile(file);
        setChanged();
        watcher.addPath(file->name());
    }
    return file->id();
}

int Project::addShape(Shapes::AbstractShape* const shape) {
    QMutexLocker locker(&mutex);
    if(!shape)
        return -1;
    isPinsPlaced_ = false;
    const int newId = shapes_.size() ? (--shapes_.end())->first + 1 : 0;
    shape->id_ = newId;
    shape->setToolTip(QString::number(newId));
    shape->setZValue(newId);
    shapes_.emplace(newId, shape); // NOTE destroy on filetree model
    App::fileModel().addShape(shape);
    setChanged();
    return newId;
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
        fileName_ = QObject::tr("Untitled") + ".g2g";
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
            "",
            QObject::tr("Board dimensions may have changed.\n"
                        "It is advisable to perform automatic placement of the pins\n"
                        "by selecting the necessary work items.\n\n"
                        "Continue saving?"),
            QMessageBox::Yes | QMessageBox::No, nullptr);
        {
            auto label(msgbx.findChild<QLabel*>());
            label->setPixmap(QIcon::fromTheme("snap-nodes-cusp").pixmap(label->size()));
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

int Project::ver() const { return ver_; }

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
