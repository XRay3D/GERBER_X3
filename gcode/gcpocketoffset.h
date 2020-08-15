#pragma once



#include "gccreator.h"

namespace GCode {
class PocketCreator : public Creator {
public:
    PocketCreator();

private:
    void createPocket(const Tool& tool, const double depth, const int steps);
    void createPocket3(const Tool& tool, const double depth);
    void createPocket2(QVector<Tool>& tools, double depth);

protected:
    void create() override; // Creator interface
};
}


