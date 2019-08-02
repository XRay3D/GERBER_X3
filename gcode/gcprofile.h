#ifndef GCPROFILE_H
#define GCPROFILE_H

#include "gccreator.h"

namespace GCode {
class ProfileCreator : public Creator {
public:
    ProfileCreator();
    ~ProfileCreator() override = default;
    void create(const GCodeParams& gcp) override; // Creator interface
private:
    void createProfile(const Tool& tool, const double depth);
};
}

#endif // GCPROFILE_H
