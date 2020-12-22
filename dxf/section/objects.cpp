#include "objects.h"

namespace Dxf {

SectionOBJECTS::SectionOBJECTS(File* file, QVector<CodeData>&& data)
    : SectionParser(std::move(data), file)
{
}

}
