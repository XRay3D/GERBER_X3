// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include <project.h>

#include "file.h"
#include "ft_model.h"
#include "graphicsview.h"

#include "shapepluginin.h"

#include <QElapsedTimer>
#include <QFileDialog>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>

const int isadfsdfg = qRegisterMetaType<FileInterface*>("FileInterface*");

QDataStream& operator<<(QDataStream& stream, const std::shared_ptr<FileInterface>& file) {
    stream << *file;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, std::shared_ptr<FileInterface>& file) {
    int type;
    stream >> type;
    if (App::filePlugins().contains(type)) {
        file.reset(App::filePlugin(type)->createFile());
        stream >> *file;
        file->addToScene();
        if (!App::project()->watcher.files().contains(file->name()))
            App::project()->watcher.addPath(file->name());
        //        qDebug() << "watcher" << App::project()->watcher.files();
    }
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const std::shared_ptr<Shapes::Shape>& shape) {
    stream << *shape;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, std::shared_ptr<Shapes::Shape>& shape) {
    int type;
    stream >> type;
    if (App::shapePlugins().contains(type)) {
        shape.reset(App::shapePlugin(type)->createShape());
        stream >> *shape;
        App::graphicsView()->scene()->addItem(shape.get());
    }
    return stream;
}

Project::Project(QObject* parent)
    : QObject(parent)
    , watcher(this) {
    connect(&watcher, &QFileSystemWatcher::fileChanged, [this](const QString& path) {
        const int id = files_[contains(path)]->id();
        if (id > -1
            && QFileInfo(path).exists()
            && QMessageBox::question(nullptr, "", tr("External file \"%1\" has changed.\nReload it into the project?").arg(QFileInfo(path).fileName()),
                   QMessageBox::Ok, QMessageBox::Cancel)
                == QMessageBox::Ok) {
            reloadFile_ = true;
            emit parseFile(path, static_cast<int>(files_[id]->type()));
        }
    });

    connect(this, &Project::addFileDbg, this, &Project::addFile, Qt::QueuedConnection);
    App::setProject(this);
}

Project::~Project() {
    App::setProject(nullptr);
}

bool Project::save(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << file.errorString();
        return false;
    }
    QDataStream out(&file);
    try {
        out << (ver_ = ProVer_6);
        // ProVer_5:
        out << isPinsPlaced_;
        // ProVer_4:
        // ProVer_3:
        out << spacingX_;
        out << spacingY_;
        out << stepsX_;
        out << stepsY_;
        // ProVer_2:
        out << home_;
        out << zero_;
        for (auto& pt : pins_)
            out << pt;
        for (auto& fl : pinsUsed_)
            out << fl;
        out << worckRect_;
        out << safeZ_;
        out << boardThickness_;
        out << copperThickness_;
        out << clearence_;
        out << plunge_;
        out << glue_;
        // ProVer_1:;
        out << files_;
        out << shapes_;
        isModified_ = false;
        emit changed();
        return true;
    } catch (...) {
        qDebug() << out.status();
    }
    return false;
}

bool Project::open(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << file.errorString();
        return false;
    }
    QDataStream in(&file);
    try {
        in >> ver_;
        switch (ver_) {
        case ProVer_6:
            [[fallthrough]];
        case ProVer_5:
            in >> isPinsPlaced_;
            [[fallthrough]];
        case ProVer_4:
            [[fallthrough]];
        case ProVer_3:
            in >> spacingX_;
            in >> spacingY_;
            in >> stepsX_;
            in >> stepsY_;
            [[fallthrough]];
        case ProVer_2:
            in >> home_;
            emit homePosChanged(home_);
            in >> zero_;
            emit zeroPosChanged(zero_);
            for (auto& pt : pins_)
                in >> pt;
            for (auto& fl : pinsUsed_)
                in >> fl;
            emit pinsPosChanged(pins_);
            in >> worckRect_;
            emit worckRectChanged(worckRect_);
            in >> safeZ_;
            in >> boardThickness_;
            in >> copperThickness_;
            in >> clearence_;
            in >> plunge_;
            in >> glue_;
            [[fallthrough]];
        case ProVer_1:;
            in >> files_;
            for (const auto& [id, filePtr] : files_)
                App::fileModel()->addFile(filePtr.get());

            in >> shapes_;
            for (const auto& [id, shPtr] : shapes_)
                App::fileModel()->addShape(shPtr.get());

            isModified_ = false;
        }
        return true;
    } catch (const QString& ex) {
        qDebug() << ex;
    } catch (const std::exception& ex) {
        qDebug() << ex.what();
    } catch (...) {
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
    for (auto& fl : pinsUsed_)
        fl = true;
    emit changed();
}

void Project::deleteFile(int id) {
    QMutexLocker locker(&mutex_);
    if (files_.contains(id)) {
        watcher.removePath(files_[id]->name());
        files_.erase(id);
        setChanged();
    } else
        qWarning() << "Error id" << id << "File not found";
}

void Project::deleteShape(int id) {
    QMutexLocker locker(&mutex_);
    try {
        if (shapes_.contains(id)) {
            shapes_.erase(id);
            setChanged();
        } else
            qWarning() << "Error id" << id << "Shape not found";
    } catch (const std::exception& ex) {
        qWarning() << ex.what();
    }
}

int Project::size() { return int(files_.size() + shapes_.size()); }

bool Project::isModified() { return isModified_; }

void Project::setModified(bool fl) { isModified_ = fl; }

QRectF Project::getBoundingRect() {
    QMutexLocker locker(&mutex_);
    Point topLeft(std::numeric_limits<Point::Type>::max(), std::numeric_limits<Point::Type>::max());
    Point botRight(std::numeric_limits<Point::Type>::min(), std::numeric_limits<Point::Type>::min());
    for (const auto& [id, filePtr] : files_) {
        if (filePtr && filePtr->itemGroup()->isVisible()) {
            for (const GraphicsItem* const item : *filePtr->itemGroup()) {
                for (const Path& path : item->paths()) {
                    for (const Point& pt : path) {
                        topLeft.x = std::min(pt.x, topLeft.x);
                        topLeft.y = std::min(pt.y, topLeft.y);
                        botRight.x = std::max(pt.x, botRight.x);
                        botRight.y = std::max(pt.y, botRight.y);
                    }
                }
            }
        }
    }
    return QRectF(topLeft, botRight);
}

QString Project::fileNames() {
    QMutexLocker locker(&mutex_);
    QString fileNames;
    for (const auto& [id, sp] : files_) {
        FileInterface* item = sp.get();
        if (sp && (item && (item->type() == FileType::Gerber || item->type() == FileType::Excellon)))
            fileNames.append(item->name()).push_back('|');
    }
    return fileNames;
}

int Project::contains(const QString& name) {
    // QMutexLocker locker(&mutex_);
    if (reloadFile_)
        return -1;
    for (const auto& [id, sp] : files_) {
        FileInterface* item = sp.get();
        if (sp && (item->type() == FileType::Gerber || item->type() == FileType::Excellon || item->type() == FileType::Dxf))
            if (QFileInfo(item->name()).fileName() == QFileInfo(name).fileName())
                return item->id();
    }
    return -1;
}

bool Project::reload(int id, FileInterface* file) {
    if (files_.contains(id)) {
        file->initFrom(files_[id].get());
        files_[id].reset(file);
        App::filePlugin(int(file->type()))->updateFileModel(file);
        setChanged();
        return true;
    }
    return false;
}

mvector<FileInterface*> Project::files(FileType type) {
    QMutexLocker locker(&mutex_);
    mvector<FileInterface*> rfiles;
    rfiles.reserve(files_.size());
    for (const auto& [id, sp] : files_) {
        if (sp && sp.get()->type() == type)
            rfiles.push_back(sp.get());
    }
    rfiles.shrink_to_fit();
    return rfiles;
}

mvector<FileInterface*> Project::files(const mvector<FileType> types) {
    QMutexLocker locker(&mutex_);
    mvector<FileInterface*> rfiles;
    rfiles.reserve(files_.size());
    for (auto type : types) {
        for (const auto& [id, sp] : files_) {
            if (sp && sp.get()->type() == type)
                rfiles.push_back(sp.get());
        }
    }
    rfiles.shrink_to_fit();
    return rfiles;
}

Shapes::Shape* Project::shape(int id) {
    QMutexLocker locker(&mutex_);
    return shapes_[id].get();
}

int Project::addFile(FileInterface* file) {
    QMutexLocker locker(&mutex_);
    if (!file)
        return -1;
    isPinsPlaced_ = false;
    reloadFile_ = false;
    file->createGi();
    file->addToScene();
    file->setVisible(true);
    const int id = contains(file->name());
    if (id > -1 && files_[id]->type() == file->type()) {
        reload(id, file);
    } else if (file->id() == -1) {
        const int newId = files_.size() ? (--files_.end())->first + 1 : 0;
        file->setId(newId);
        files_.emplace(newId, file);
        App::fileModel()->addFile(file);
        setChanged();
        watcher.addPath(file->name());
        //        qDebug() << "watcher" << watcher.files();
    }
    return file->id();
}

int Project::addShape(Shapes::Shape* const shape) {
    QMutexLocker locker(&mutex_);
    if (!shape)
        return -1;
    isPinsPlaced_ = false;
    const int newId = shapes_.size() ? (--shapes_.end())->first + 1 : 0;
    shape->id_ = newId;
    shape->setToolTip(QString::number(newId));
    shape->setZValue(newId);
    shapes_.emplace(newId, shape);
    App::fileModel()->addShape(shape);
    setChanged();
    return newId;
}

bool Project::contains(FileInterface* file) {
    for (const auto& [id, sp] : files_)
        if (sp.get() == file)
            return true;
    return false;
}

QString Project::name() { return name_; }

void Project::setName(const QString& name) {
    setUntitled(name.isEmpty());
    if (isUntitled_)
        name_ = QObject::tr("Untitled.g2g");
    else
        name_ = name;
}

void Project::setChanged() {
    isModified_ = true;
    changed();
}

bool Project::pinsPlacedMessage() {

    if (isPinsPlaced_ == false) {
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

double Project::spaceX() const { return spacingX_; }
void Project::setSpaceX(double value) {
    spacingX_ = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

double Project::spaceY() const { return spacingY_; }
void Project::setSpaceY(double value) {
    spacingY_ = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

uint Project::stepsX() const { return stepsX_; }
void Project::setStepsX(uint value) {
    stepsX_ = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

uint Project::stepsY() const { return stepsY_; }
void Project::setStepsY(uint value) {
    stepsY_ = value;
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
