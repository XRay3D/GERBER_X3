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
#pragma once
//#include "gctypes.h"
#include "gcutils.h"
#include "interfaces/file.h"

class MainWindow;
class GcPathItem;

namespace GCode {

class File : private GCUtils, public FileInterface {
    friend class ::GcPathItem;
    friend class ::MainWindow;

public:
    explicit File();
    explicit File(const Pathss& toolPathss, const GCodeParams& gcp, const Paths& pocketPaths = {});
    bool save(const QString& name);
    GCodeType gtype() const;
    FileType type() const override { return FileType::GCode; }
    QIcon icon() const { return m_icon; }

private:
    ////////////////////////////////////////
    Paths m_pocketPaths; /////
    Pathss m_toolPathss; /////
    const GCodeParams m_gcp; ////

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
    QIcon m_icon;
    //////////////
    void saveDrill(const QPointF& offset);

    void saveLaserPocket(const QPointF& offset);
    void saveMillingPocket(const QPointF& offset);

    void saveLaserProfile(const QPointF& offset);
    void saveMillingProfile(const QPointF& offset);

    void saveMillingRaster(const QPointF& offset);

    void saveLaserHLDI(const QPointF& offset);
    mvector<QSharedPointer<QColor>> debugColor;

    // FileInterface interfaces
protected:
    virtual Paths merge() const override { return {}; }
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

public:
    void createGi() override;
    void initFrom(FileInterface* file) override { }
    FileTree::Node* node() override;

    mvector<QString> gCodeText() const;
    Tool getTool() const;
    const GCodeParams& gcp() const;
};
}
