#ifndef THERMAL_H
#define THERMAL_H

#include "gccreator.h"

namespace Gerber {
class File;
}

namespace GCode {
class ThermalCreator : public Creator {
public:
    ThermalCreator();
    ~ThermalCreator() override = default;

private:
    void createThermal(Gerber::File* file, const Tool& tool, const double depth);

protected:
    void create(const GCodeParams& gcp) override; // Creator interface
};
}
#endif // THERMAL_H
