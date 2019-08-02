#ifndef POCKET_H
#define POCKET_H

#include "gccreator.h"

namespace GCode {
class PocketCreator : public Creator {
public:
    PocketCreator();
    void create(const GCodeParams& gcp) override; // Creator interface
private:
    void createRaster(const Tool& tool, const double depth, const double angle, const int pPass);
    void createPocket(const Tool& tool, const double depth, const int steps);
    void createPocket2(const QPair<Tool, Tool>& tool, double depth);
};
}

#endif // POCKET_H
