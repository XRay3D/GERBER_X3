#pragma once
//#ifndef POCKET_H
//#define POCKET_H

#include "gccreator.h"

namespace GCode {
class PocketCreator : public Creator {
public:
    PocketCreator();

private:
    void createFixedSteps(const Tool& tool, const double depth, const int steps);
    void createStdFull(const Tool& tool, const double depth);
    void createMultiTool(QVector<Tool>& tools, double depth);

protected:
    void create() override; // Creator interface
};
}

//#endif // POCKET_H
