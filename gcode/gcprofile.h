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

protected:
    void create() override; // Creator interface
    GCodeType type() override { return Profile; }
};
}
