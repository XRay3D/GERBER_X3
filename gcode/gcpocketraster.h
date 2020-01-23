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
    void createRaster(const Tool& tool, const double depth, const double angle, const int prPass);
    void createRaster2(const Tool& tool, const double depth, const double angle, const int prPass);
};
}
#endif // LASERCREATOR_H
