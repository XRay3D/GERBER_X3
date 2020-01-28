#ifndef LASERCREATOR_H
#define LASERCREATOR_H

#include "gccreator.h"
namespace GCode {
class RasterCreator : public Creator {
public:
    RasterCreator();
    ~RasterCreator() override = default;

    // Creator interface
protected:
    void create() override; // Creator interface

private:
    enum {
        NoProfilePass,
        First,
        Last
    };

    void createRaster(const Tool& tool, const double depth, const double angle, const int prPass);
    void createRaster2(const Tool& tool, const double depth, const double angle, const int prPass);
    //    Paths& sortBE(Paths& src);
    void sortBE2(Paths& src);
    IntRect rect;
    IntPoint p0;
};
}
#endif // LASERCREATOR_H
