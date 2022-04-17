// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include <project.h>

#include "file.h"
#include "ft_model.h"
#include "mainwindow.h"
#include "shape.h"
#include "shapepluginin.h"

#include <scene.h>

#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileSystemWatcher>
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
        App::project()->watcher->addPath(file->name());
        qDebug() << App::project()->watcher->files();
    }
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const std::shared_ptr<ShapeInterface>& shape) {
    stream << *shape;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, std::shared_ptr<ShapeInterface>& shape) {
    int type;
    stream >> type;
    if (App::shapePlugins().contains(type)) {
        shape.reset(App::shapePlugin(type)->createShape());
        stream >> *shape;
        App::scene()->addItem(shape.get());
    }
    return stream;
}

Project::Project(QObject* parent)
    : QObject(parent)
    , watcher(new QFileSystemWatcher(this)) {
    connect(watcher, &QFileSystemWatcher::fileChanged, [this](const QString& path) {
        const int id = m_files[contains(path)]->id();
        if (id > -1
            && QFileInfo(path).exists()
            && QMessageBox::question(nullptr, "", tr("External file \"%1\" has changed.\n"
                                                     "Reload it into the project?")
                                                      .arg(QFileInfo(path).fileName()),
                   QMessageBox::Ok, QMessageBox::Cancel)
                == QMessageBox::Ok) {
            m_reloadFile = true;
            emit parseFile(path, static_cast<int>(m_files[id]->type()));
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
        out << (m_ver = ProVer_5);
        // ProVer_5:
        out << m_isPinsPlaced;
        // ProVer_4:
        // ProVer_3:
        out << m_spacingX;
        out << m_spacingY;
        out << m_stepsX;
        out << m_stepsY;
        // ProVer_2:
        out << m_home;
        out << m_zero;
        for (auto& pt : m_pins)
            out << pt;
        for (auto& fl : m_pinsUsed)
            out << fl;
        out << m_worckRect;
        out << m_safeZ;
        out << m_boardThickness;
        out << m_copperThickness;
        out << m_clearence;
        out << m_plunge;
        out << m_glue;
        // ProVer_1:;
        out << m_files;
        out << m_shapes;
        m_isModified = false;
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
        in >> m_ver;
        switch (m_ver) {
        case ProVer_5:
            in >> m_isPinsPlaced;
            [[fallthrough]];
        case ProVer_4:
            [[fallthrough]];
        case ProVer_3:
            in >> m_spacingX;
            in >> m_spacingY;
            in >> m_stepsX;
            in >> m_stepsY;
            [[fallthrough]];
        case ProVer_2:
            in >> m_home;
            emit homePosChanged(m_home);
            in >> m_zero;
            emit zeroPosChanged(m_zero);
            for (auto& pt : m_pins)
                in >> pt;
            for (auto& fl : m_pinsUsed)
                in >> fl;
            emit pinsPosChanged(m_pins);
            in >> m_worckRect;
            emit worckRectChanged(m_worckRect);
            in >> m_safeZ;
            in >> m_boardThickness;
            in >> m_copperThickness;
            in >> m_clearence;
            in >> m_plunge;
            in >> m_glue;
            [[fallthrough]];
        case ProVer_1:;
            in >> m_files;
            for (const auto& [id, filePtr] : m_files)
                App::fileModel()->addFile(filePtr.get());

            in >> m_shapes;
            for (const auto& [id, shPtr] : m_shapes)
                App::fileModel()->addShape(shPtr.get());

            m_isModified = false;
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
    m_isPinsPlaced = false;
    m_isModified = false;
    for (auto& fl : m_pinsUsed)
        fl = true;
    emit changed();
}

void Project::deleteFile(int id) {
    QMutexLocker locker(&m_mutex);
    if (m_files.contains(id)) {
        watcher->removePath(m_files[id]->name());
        m_files.erase(id);
        setChanged();
    } else
        qWarning() << "Error id" << id << "File not found";
}

void Project::deleteShape(int id) {
    QMutexLocker locker(&m_mutex);
    try {
        if (m_shapes.contains(id)) {
            m_shapes.erase(id);
            setChanged();
        } else
            qWarning() << "Error id" << id << "Shape not found";
    } catch (const std::exception& ex) {
        qWarning() << ex.what();
    }
}

int Project::size() { return int(m_files.size() + m_shapes.size()); }

bool Project::isModified() { return m_isModified; }

void Project::setModified(bool fl) { m_isModified = fl; }

QRectF Project::getBoundingRect() {
    QMutexLocker locker(&m_mutex);
    IntPoint topLeft(std::numeric_limits<cInt>::max(), std::numeric_limits<cInt>::max());
    IntPoint botRight(std::numeric_limits<cInt>::min(), std::numeric_limits<cInt>::min());
    for (const auto& [id, filePtr] : m_files) {
        if (filePtr && filePtr->itemGroup()->isVisible()) {
            for (const GraphicsItem* const item : *filePtr->itemGroup()) {
                for (const Path& path : item->paths()) {
                    for (const IntPoint& pt : path) {
                        topLeft.X = qMin(pt.X, topLeft.X);
                        topLeft.Y = qMin(pt.Y, topLeft.Y);
                        botRight.X = qMax(pt.X, botRight.X);
                        botRight.Y = qMax(pt.Y, botRight.Y);
                    }
                }
            }
        }
    }
    return QRectF(topLeft, botRight);
}

QString Project::fileNames() {
    QMutexLocker locker(&m_mutex);
    QString fileNames;
    for (const auto& [id, sp] : m_files) {
        FileInterface* item = sp.get();
        if (sp && (item && (item->type() == FileType::Gerber || item->type() == FileType::Excellon)))
            fileNames.append(item->name()).push_back('|');
    }
    return fileNames;
}

int Project::contains(const QString& name) {
    // QMutexLocker locker(&m_mutex);
    if (m_reloadFile)
        return -1;
    for (const auto& [id, sp] : m_files) {
        FileInterface* item = sp.get();
        if (sp && (item->type() == FileType::Gerber || item->type() == FileType::Excellon || item->type() == FileType::Dxf))
            if (QFileInfo(item->name()).fileName() == QFileInfo(name).fileName())
                return item->id();
    }
    return -1;
}

bool Project::reload(int id, FileInterface* file) {
    if (m_files.contains(id)) {
        file->initFrom(m_files[id].get());
        m_files[id].reset(file);
        App::filePlugin(int(file->type()))->updateFileModel(file);
        setChanged();
        return true;
    }
    return false;
}

mvector<FileInterface*> Project::files(FileType type) {
    QMutexLocker locker(&m_mutex);
    mvector<FileInterface*> rfiles;
    rfiles.reserve(m_files.size());
    for (const auto& [id, sp] : m_files) {
        if (sp && sp.get()->type() == type)
            rfiles.push_back(sp.get());
    }
    rfiles.shrink_to_fit();
    return rfiles;
}

mvector<FileInterface*> Project::files(const mvector<FileType> types) {
    QMutexLocker locker(&m_mutex);
    mvector<FileInterface*> rfiles;
    rfiles.reserve(m_files.size());
    for (auto type : types) {
        for (const auto& [id, sp] : m_files) {
            if (sp && sp.get()->type() == type)
                rfiles.push_back(sp.get());
        }
    }
    rfiles.shrink_to_fit();
    return rfiles;
}

ShapeInterface* Project::shape(int id) {
    QMutexLocker locker(&m_mutex);
    return m_shapes[id].get();
}

int Project::addFile(FileInterface* file) {
    QMutexLocker locker(&m_mutex);
    if (!file)
        return -1;
    m_isPinsPlaced = false;
    m_reloadFile = false;
    file->createGi();
    file->addToScene();
    file->setVisible(true);
    const int id = contains(file->name());
    if (id > -1 && m_files[id]->type() == file->type()) {
        reload(id, file);
    } else if (file->id() == -1) {
        const int newId = m_files.size() ? (--m_files.end())->first + 1 : 0;
        file->setId(newId);
        m_files.emplace(newId, file);
        App::fileModel()->addFile(file);
        setChanged();
        watcher->addPath(file->name());
        qDebug() << watcher->files();
    }
    return file->id();
}

int Project::addShape(ShapeInterface* const shape) {
    QMutexLocker locker(&m_mutex);
    if (!shape)
        return -1;
    m_isPinsPlaced = false;
    const int newId = m_shapes.size() ? (--m_shapes.end())->first + 1 : 0;
    shape->m_giId = newId;
    shape->setToolTip(QString::number(newId));
    shape->setZValue(newId);
    m_shapes.emplace(newId, shape);
    App::fileModel()->addShape(shape);
    setChanged();
    return newId;
}

bool Project::contains(FileInterface* file) {
    for (const auto& [id, sp] : m_files)
        if (sp.get() == file)
            return true;
    return false;
}

QString Project::name() { return m_name; }

void Project::setName(const QString& name) {
    setUntitled(name.isEmpty());
    if (m_isUntitled)
        m_name = QObject::tr("Untitled.g2g");
    else
        m_name = name;
}

void Project::setChanged() {
    m_isModified = true;
    changed();
}

bool Project::pinsPlacedMessage() {

    if (m_isPinsPlaced == false) {
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

bool Project::isUntitled() { return m_isUntitled; }

bool Project::isPinsPlaced() const { return m_isPinsPlaced; }

void Project::setUntitled(bool value) {
    m_isUntitled = value;
    emit layoutFrameUpdate();
    setChanged();
}

double Project::spaceX() const { return m_spacingX; }
void Project::setSpaceX(double value) {
    m_spacingX = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

double Project::spaceY() const { return m_spacingY; }
void Project::setSpaceY(double value) {
    m_spacingY = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

uint Project::stepsX() const { return m_stepsX; }
void Project::setStepsX(uint value) {
    m_stepsX = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

uint Project::stepsY() const { return m_stepsY; }
void Project::setStepsY(uint value) {
    m_stepsY = value;
    emit layoutFrameUpdate(true);
    setChanged();
}

QRectF Project::worckRect() const { return m_worckRect; }
void Project::setWorckRect(const QRectF& worckRect) {
    m_worckRect = worckRect;
    m_isPinsPlaced = true;
    emit layoutFrameUpdate();
    setChanged();
}

QPointF Project::homePos() const { return m_home; }
void Project::setHomePos(const QPointF& pos) {
    m_home = pos;
    setChanged();
}

QPointF Project::zeroPos() const { return m_zero; }
void Project::setZeroPos(const QPointF& pos) {
    m_zero = pos;
    setChanged();
}

const QPointF* Project::pinsPos() const { return m_pins; }
void Project::setPinsPos(const QPointF pos[4]) {
    m_pins[0] = pos[0];
    m_pins[1] = pos[1];
    m_pins[2] = pos[2];
    m_pins[3] = pos[3];
    setChanged();
}

bool Project::pinUsed(int idx) const { return m_pinsUsed[idx]; }
void Project::setPinUsed(bool used, int idx) {
    m_pinsUsed[idx] = used;
    setChanged();
}

int Project::ver() const { return m_ver; }

double Project::safeZ() const { return m_safeZ; }
void Project::setSafeZ(double safeZ) {
    m_safeZ = safeZ;
    setChanged();
}

double Project::boardThickness() const { return m_boardThickness; }
void Project::setBoardThickness(double boardThickness) {
    m_boardThickness = boardThickness;
    setChanged();
}

double Project::copperThickness() const { return m_copperThickness; }
void Project::setCopperThickness(double copperThickness) {
    m_copperThickness = copperThickness;
    setChanged();
}

double Project::clearence() const { return m_clearence; }
void Project::setClearence(double clearence) {
    m_clearence = clearence;
    setChanged();
}

double Project::plunge() const { return m_plunge; }
void Project::setPlunge(double plunge) {
    m_plunge = plunge;
    setChanged();
}

double Project::glue() const { return m_glue; }
void Project::setGlue(double glue) {
    m_glue = glue;
    setChanged();
}
