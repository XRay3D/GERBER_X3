#pragma once

#include <QObject>
#include <codedata.h>

#include <section/sectionparser.h>

#ifndef TABLE_ITEM_H
#define TABLE_ITEM_H
namespace Dxf {
struct TableItem {
    TableItem(SectionParser* sp);
    virtual ~TableItem() { }

    QVector<CodeData> data;
    SectionParser* sp = nullptr;
    virtual void parse(CodeData& code) = 0;
    static int toType(const QString& key);

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

    virtual Type type() const = 0;

    Q_ENUM(Type)
    Q_GADGET
};
}
#endif // TABLE_ITEM_H

#include "appid.h"
#include "block_record.h"
#include "dimstyle.h"
#include "layer.h"
#include "ltype.h"
#include "style.h"
#include "tableitem.h"
#include "ucs.h"
#include "view.h"
#include "vport.h"
