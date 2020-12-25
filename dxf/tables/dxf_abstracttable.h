#pragma once

#include "dxf_codedata.h"
#include "section/dxf_sectionparser.h"
#include <QObject>

namespace Dxf {

struct AbstractTable {
    Q_GADGET

public:
    AbstractTable(SectionParser* sp);
    virtual ~AbstractTable() = default;

    virtual void parse(CodeData& code) = 0;
    enum Type {
        APPID,
        BLOCK_RECORD,
        DIMSTYLE,
        LAYER,
        LTYPE,
        STYLE,
        UCS,
        VIEW,
        VPORT,
    };
    Q_ENUM(Type)
    static Type toType(const QString& key);

    virtual Type type() const = 0;

    Codes data;
    SectionParser* sp = nullptr;
    File* m_file = nullptr;

    File* file() const { return sp == nullptr ? m_file : sp->file; }
};

}
