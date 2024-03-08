// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thread.h"
#include "app.h"
// #include "gc_gi_bridge.h"
#include "gi_gcpath.h"
// #include "graphicsview.h"

#include <execution>

namespace Thread {

void Creator::create() {
    // WARNING App::fileTreeView().closeFiles();
    createThread(gcp_.tools.front(), gcp_.params[GCode::Params::Depth].toDouble());
}

void Creator::createThread(const Tool& tool, const double depth) {
    do {

        //        toolDiameter = tool.getDiameter(depth);

        //        const double dOffset = ((gcp_.side() == GCode::Outer) ? +toolDiameter : -toolDiameter) * 0.5 * uScale;

        //        if(gcp_.side() == GCode::On) {
        //            if(gcp_.params[TrimmingOpenPaths].toBool())
        //                trimmingOpenPaths(openSrcPaths);
        //            returnPs = std::move(closedSrcPaths);
        //        } else {
        //            if(closedSrcPaths.size()) {
        //                ClipperOffset offset;
        //                for(Paths& paths: groupedPaths(GCode::Grouping::Copper))
        //                    offset.AddPaths(paths, JoinType::Round, EndType::Polygon);
        //                returnPs = offset.Execute(dOffset);
        //            }
        //            if(openSrcPaths.size()) {
        //                ClipperOffset offset;
        //                offset.AddPaths(openSrcPaths, JoinType::Round, EndType::Round);
        //                openSrcPaths = offset.Execute(dOffset);
        //                if(!openSrcPaths.empty())
        //                    returnPs.append(openSrcPaths);
        //            }
        //        }

        //        if(returnPs.empty() && openSrcPaths.empty())
        //            break;

        //        reorder();

        //        if(gcp_.side() == GCode::On && openSrcPaths.size()) {
        //            returnPss.reserve(returnPss.size() + openSrcPaths.size());
        //            mergePaths(openSrcPaths);
        //            sortBeginEnd(openSrcPaths);
        //            for(auto&& path: openSrcPaths)
        //                returnPss.push_back({std::move(path)});
        //        }

        //        makeBridges();

        //        if(gcp_.params.contains(TrimmingCorners) && gcp_.params[TrimmingCorners].toInt())
        //            cornerTrimming();

        //        if(returnPss.empty())
        //            break;

        //        file_ = new File{std::move(gcp_), std::move(returnPss));
        //        file_->setFileName(tool.nameEnc());
        //        emit fileReady(file_);
        //        return;
    } while(0);
    emit fileReady(nullptr);
}

////////////////////////////////////////////////////////

File::File()
    : GCode::File() { }

File::File(GCode::Params&& gcp, Pathss&& toolPathss)
    : GCode::File(std::move(gcp), std::move(toolPathss)) {
    if(gcp_.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void File::genGcodeAndTile() {
    //    const QRectF rect = App::project().worckRect();
    //    for (size_t x = 0; x < App::project().stepsX(); ++x) {
    //        for (size_t y = 0; y < App::project().stepsY(); ++y) {
    //            const QPointF offset((rect.width() + App::project().spaceX()) * x, (rect.height() + App::project().spaceY()) * y);

    //            if (toolType() == Tool::Laser)
    //                saveLaserThread(offset);
    //            else
    //                saveMillingThread(offset);

    //            if (gcp_.params.contains(GCode::Params::NotTile))
    //                return;
    //        }
    //    }
}

void File::createGi() {

    Gi::Item* item;
    for(const Paths& paths: toolPathss_) {
        item = new Gi::GcPath{paths, this};
        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }

    for(size_t i{}; const Paths& paths: toolPathss_) {
        item = new Gi::GcPath{toolPathss_[i], this};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for(size_t j = 0; j < paths.size() - 1; ++j)
            g0path_.push_back({paths[j].back(), paths[j + 1].front()});
        if(i < toolPathss_.size() - 1)
            g0path_.push_back({toolPathss_[i].back().back(), toolPathss_[++i].front().front()});
    }

    item = new Gi::GcPath{g0path_};
    //    item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);

    itemGroup()->setVisible(true);
}

} // namespace Thread
