/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "gccreator.h"

namespace GCode {
class ProfileCreator : public Creator {
public:
    ProfileCreator();
    ~ProfileCreator() override = default;

private:
    void createProfile(const Tool& tool, const double depth);
    void strip();

    enum NodeType {
        ntAny,
        ntOpen,
        ntClosed
    };
    void reorder();

    void addPolyNodeToPaths(const PolyNode& polynode, NodeType nodetype, Paths& paths);
    //------------------------------------------------------------------------------
    void polyTreeToPaths(const PolyTree& polytree, Paths& paths);
    //-----------------------------------------------------------------------------
    void closedPathsFromPolyTree(const PolyTree& polytree, Paths& paths);
    //------------------------------------------------------------------------------
    void openPathsFromPolyTree(const PolyTree& polytree, Paths& paths);

protected:
    void create() override; // Creator interface
    GCodeType type() override;
};
}
