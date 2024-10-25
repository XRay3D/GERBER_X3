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

#include "datastream.h"
#include "mvector.h"
#include "utils.h"

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
    CurrentVer = ProVer_7,
};

namespace GCode {
class File;
}

namespace Gi {
class Item;
}

namespace Shapes {
class AbstractShape;
}

class AbstractFile;
class QFile;
class QFileSystemWatcher;

using FilesMap = std::map<int, std::shared_ptr<AbstractFile>>;
// using ShapesMap = std::map<int, std::shared_ptr<Shapes::AbstractShape>>;
using ShapesMap = std::map<int, Shapes::AbstractShape*>;
using ItemMap = std::map<int, Gi::Item*>;

class Project : public QObject {
    Q_OBJECT
    friend QDataStream& operator>>(QDataStream& stream, std::shared_ptr<AbstractFile>& file);

public:
    explicit Project(QObject* parent = nullptr);
    ~Project();

    // AbstractFile
    template <typename T = AbstractFile>
    T* file(int32_t id) {
        QMutexLocker locker(&mutex);
        if(files_.contains(id))
            return static_cast<T*>(files_[id].get());
        return nullptr;
    }

    template <typename T = AbstractFile>
    mvector<T*> files() {
        QMutexLocker locker(&mutex);
        mvector<T*> rfiles;
        for(const auto& [id, sp]: files_) {
            T* file = dynamic_cast<T*>(sp.get());
            if(file)
                rfiles.push_back(file);
        }
        return rfiles;
    }

    template <typename T>
    mvector<T*> count() {
        QMutexLocker locker(&mutex);
        int count = 0;
        for(const auto& [id, sp]: files_)
            if(dynamic_cast<T*>(sp.get()))
                ++count;
        return count;
    }

    int addFile(AbstractFile* const file);
    int addFile(GCode::File* const file);
    bool contains(AbstractFile* file);
    mvector<AbstractFile*> files(int type);
    mvector<AbstractFile*> files(const mvector<int>& types);
    void deleteFile(int32_t id);
    //    QString fileNames();
    int contains(const QString& name);

    // AbstractShape
    int addShape(Shapes::AbstractShape* const shape);
    Shapes::AbstractShape* shape(int32_t id);
    void deleteShape(int32_t id);

    // Item
    int addItem(Gi::Item* const item);
    Gi::Item* Item(int32_t id);
    void deleteItem(int32_t id);

    // Shape
    int makeShapeCircle(const QPointF& center, const QPointF& rad);
    int makeShapeRectangle(const QPointF& center, const QPointF& rect);

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
    void addFileDbg(GCode::File* file);
    // File Watcher
    void reloadFile(const QString& filename, int type);

private:
    bool reload(int32_t id, AbstractFile* file);

    // File Watcher
    QFileSystemWatcher watcher;
    bool reloadFile_ = false;

    int ver_;

    FilesMap files_;
    ShapesMap shapes_;
    ItemMap items_;

    QMutex mutex;

    QString fileName_;
    bool isModified_ = false;
    bool isUntitled_ = true;
    bool isPinsPlaced_ = false;
    bool pinsUsed_[4]{true, true, true, true};
    QPointF pins_[4];
    QPointF home_;
    QPointF zero_;
    QRectF worckRect_;

    double safeZ_{};
    double boardThickness_{};
    double copperThickness_{};
    double clearence_{};
    double plunge_{};
    double glue_{};

    struct Tailing {
        double spacingX{};
        double spacingY{};
        uint stepsX{1};
        uint stepsY{1};
        SERIALIZE_POD(Tailing)
    } tailing;
};
