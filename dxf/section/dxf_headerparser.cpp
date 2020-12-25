#include "dxf_headerparser.h"
#include "dxf_file.h"
#include <QTreeWidget>

namespace Dxf {
SectionHEADER::SectionHEADER(File* file, Codes::iterator from, Codes::iterator to)
    : SectionParser(from, to, file)
    , header(file->header())
{
}

void SectionHEADER::parse()
{
    QString key;
    do {
        CodeData code(nextCode());
        if (code == "ENDSEC")
            continue;
        if (code.type() == CodeData::String && QString(code).startsWith('$')) {
            key = code.string();
        } else if (!key.isEmpty()) {
            header[key][code.code()] = code.value();
        }
    } while (hasNext());
}
}
