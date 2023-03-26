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
#pragma once

#include "mvector.h"

#include <QFileSystemWatcher>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QRectF>
#include <memory>

enum FileVersion {
    ProVer_1 = 1,
    ProVer_2,
    ProVer_3,
    ProVer_4,
    ProVer_5,
    ProVer_6,
    ProVer_7,
};

class FileInterface;
namespace Shapes {
class Shape;
}
class QFile;
class QFileSystemWatcher;

using FilesMap = std::map<int, std::shared_ptr<FileInterface>>;
using ShapesMap = std::map<int, std::shared_ptr<Shapes::Shape>>;

class Project : public QObject {
    Q_OBJECT
    friend QDataStream& operator>>(QDataStream& stream, std::shared_ptr<FileInterface>& file);

public:
    explicit Project(QObject* parent = nullptr);
    ~Project();

    // FileInterface
    template <typename T = FileInterface>
    T* file(int id) {
        QMutexLocker locker(&mutex_);
        if (files_.contains(id))
            return static_cast<T*>(files_[id].get());
        return nullptr;
    }

    template <typename T = FileInterface>
    mvector<T*> files() {
        QMutexLocker locker(&mutex_);
        mvector<T*> rfiles;
        for (const auto& [id, sp] : files_) {
            T* file = dynamic_cast<T*>(sp.get());
            if (file)
                rfiles.push_back(file);
        }
        return rfiles;
    }

    template <typename T>
    mvector<T*> count() {
        QMutexLocker locker(&mutex_);
        int count = 0;
        for (const auto& [id, sp] : files_) {
            if (dynamic_cast<T*>(sp.get()))
                ++count;
        }
        return count;
    }

    int addFile(FileInterface* const file);
    bool contains(FileInterface* file);
    mvector<FileInterface*> files(int type);
    mvector<FileInterface*> files(const mvector<int> types);
    void deleteFile(int id);
    QString fileNames();
    int contains(const QString& name);

    // Shape
    int addShape(Shapes::Shape* const shape);
    Shapes::Shape* shape(int id);
    void deleteShape(int id);

    // Project
    bool save(const QString& fileName);
    bool open(const QString& fileName);
    void close();

    int ver() const;

    int size();

    bool isModified();
    void setModified(bool fl);

    QString name();
    void setName(const QString& name);
    void setChanged();
    bool pinsPlacedMessage();

    bool isUntitled();
    bool isPinsPlaced() const;
    void setUntitled(bool value);

    // Bounding Rect
    QRectF getBoundingRect();

    double spaceX() const;
    void setSpaceX(double value);

    double spaceY() const;
    void setSpaceY(double value);

    uint stepsX() const;
    void setStepsX(uint value);

    uint stepsY() const;
    void setStepsY(uint value);

    double safeZ() const;
    void setSafeZ(double safeZ);

    double boardThickness() const;
    void setBoardThickness(double boardThickness);

    double copperThickness() const;
    void setCopperThickness(double copperThickness);

    double clearence() const;
    void setClearence(double clearence);

    double plunge() const;
    void setPlunge(double plunge);

    double glue() const;
    void setGlue(double glue);

    QRectF worckRect() const;
    void setWorckRect(const QRectF& worckRect);

    QPointF homePos() const;
    void setHomePos(const QPointF& pos);

    QPointF zeroPos() const;
    void setZeroPos(const QPointF& pos);

    const QPointF* pinsPos() const;
    void setPinsPos(const QPointF pos[4]);

    bool pinUsed(int idx) const;
    void setPinUsed(bool used, int idx);

signals:
    void changed();
    void homePosChanged(const QPointF&);
    void zeroPosChanged(const QPointF&);
    void pinsPosChanged(QPointF*);
    void worckRectChanged(const QRectF&);
    void layoutFrameUpdate(bool = false);
    // need for debuging
    void addFileDbg(FileInterface* file);
    // File Watcher
    void parseFile(const QString& filename, int type);

private:
    bool reload(int id, FileInterface* file);

    // File Watcher
    QFileSystemWatcher watcher;
    bool reloadFile_ = false;

    int ver_;

    FilesMap files_;
    ShapesMap shapes_;
    QMutex mutex_;
    QString fileName_;
    QString name_;
    bool isModified_ = false;
    bool isUntitled_ = true;
    bool isPinsPlaced_ = false;

    QPointF home_;
    QPointF zero_;
    QPointF pins_[4];
    bool pinsUsed_[4] {true, true, true, true};
    QRectF worckRect_;

    double safeZ_ {};
    double boardThickness_ {};
    double copperThickness_ {};
    double clearence_ {};
    double plunge_ {};
    double glue_ {};

    double spacingX_ {};
    double spacingY_ {};
    uint stepsX_ {1};
    uint stepsY_ {1};
};
