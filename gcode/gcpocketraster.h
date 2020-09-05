#pragma once

#include "gccreator.h"
namespace GCode {
class RasterCreator : public Creator {
public:
    RasterCreator();
    ~RasterCreator() override = default;

    // Creator interface
protected:
    void create() override; // Creator interface
    GCodeType type() override { return Raster; }

private:
    enum {
        NoProfilePass,
        First,
        Last
    };

    void createRaster(const Tool& tool, const double depth, const double angle, const int prPass);
    void createRaster2(const Tool& tool, const double depth, const double angle, const int prPass);
    void addAcc(Paths& src, const cInt accDistance);

    IntRect rect;
};
}
