#ifndef POCKET_H
#define POCKET_H

#include "gccreator.h"

namespace GCode {
class PocketCreator : public Creator {
public:
    PocketCreator();

private:
    void createRaster(const Tool& tool, const double depth, const double angle, const int prPass);
    void createPocket(const Tool& tool, const double depth, const int steps);
    void createPocket2(const QPair<Tool, Tool>& tool, double depth, double minArea);

protected:
    void create() override; // Creator interface
};
}

#endif // POCKET_H
