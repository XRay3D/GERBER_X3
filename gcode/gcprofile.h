#ifndef GCPROFILE_H
#define GCPROFILE_H

#include "gccreator.h"

namespace GCode {
class ProfileCreator : public Creator {
public:
    ProfileCreator();
    ~ProfileCreator() override = default;

private:
    void createProfile(const Tool& tool, const double depth);
protected:
    void create() override; // Creator interface
};
}

#endif // GCPROFILE_H
