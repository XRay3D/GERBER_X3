#include "classes.h"

namespace Dxf {

SectionCLASSES::SectionCLASSES(File* file, QVector<CodeData>&& data)
    : SectionParser(std::move(data), file)
{
}

}
