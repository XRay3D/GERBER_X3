#include "thumbnailimage.h"

namespace Dxf {

SectionTHUMBNAILIMAGE::SectionTHUMBNAILIMAGE(File* file, QVector<CodeData>&& data)
    : SectionParser(std::move(data), file)
{
}

}
