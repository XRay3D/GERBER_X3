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

#include "file.h"
#include "gc_utils.h"

class MainWindow;
class GiGcPath;

namespace GCode {

struct GCObj : QObject {
    //    Q_OBJECT
    // public:
    //    GCObj() { }
    //    virtual ~GCObj() { }
};

class File : public GCFile {
    friend class ::GiGcPath;
    friend class ::MainWindow;

public:
    explicit File();
    explicit File(const Pathss& toolPathss, GCodeParams&& gcp, const Paths& pocketPaths = {});
    bool save(const QString& name);
    GCodeType gtype() const;
    uint32_t type() const override { return FileType::GCode; }
    QIcon icon() const { return icon_; }

private:
    ////////////////////////////////////////
    Paths pocketPaths_; /////
    Pathss toolPathss_; /////

    mvector<mvector<QPolygonF>> normalizedPathss(const QPointF& offset);
    mvector<QPolygonF> normalizedPaths(const QPointF& offset, const Paths& paths_ = {});

    void initSave();
    void genGcodeAndTile();
    void addInfo();
    void startPath(const QPointF& point);
    void endPath();
    void statFile();
    void endFile();

    void createGiDrill();
    void createGiPocket();
    void createGiProfile();
    void createGiRaster();
    void createGiLaser();
    QIcon icon_;
    //////////////
    void saveDrill(const QPointF& offset);

    void saveLaserPocket(const QPointF& offset);
    void saveMillingPocket(const QPointF& offset);

    void saveLaserProfile(const QPointF& offset);
    void saveMillingProfile(const QPointF& offset);

    void saveMillingRaster(const QPointF& offset);

    void saveLaserHLDI(const QPointF& offset);
    mvector<QSharedPointer<QColor>> debugColor;

    // AbstractFile interfaces
protected:
    virtual Paths merge() const override { return {}; }
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

public:
    void createGi() override;
    void initFrom(AbstractFile* file) override { }
    FileTree_::Node* node() override;

    mvector<QString> gCodeText() const;
    Tool getTool() const;
    const GCodeParams& gcp() const;
};

} // namespace GCode
