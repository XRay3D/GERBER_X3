#pragma once

#include "datastream.h"
#include "dxf_codedata.h"
#include <QDebug>
#include <QObject>

namespace Dxf {

class File;

struct SectionParser {
    Q_GADGET

public:
    SectionParser(Codes::iterator from, Codes::iterator to, Codes::iterator it, File* file);
    SectionParser(Codes::iterator from, Codes::iterator to, File* file);
    virtual ~SectionParser() = default;

    virtual void parse() = 0;

    enum SectionType {
        NullType = -1,
        HEADER,
        CLASSES,
        TABLES,
        BLOCKS,
        ENTITIES,
        OBJECTS,
        THUMBNAILIMAGE,
    };
    Q_ENUM(SectionType)
    const SectionType type;
    static SectionType toType(const QString& key);

    mutable int counter = 0;

    const Codes::iterator from;
    const Codes::iterator to;
    mutable Codes::iterator it;

    File* file;

    const CodeData& nextCode() const;
    const CodeData& prevCode() const;

    bool hasNext() const;
    bool hasPrev() const;

    friend QDebug operator<<(QDebug debug, const SectionParser& c);
};
}
