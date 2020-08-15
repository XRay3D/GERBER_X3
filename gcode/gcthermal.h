#pragma once



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
    void create() override; // Creator interface
};
}

