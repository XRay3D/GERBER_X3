#include "tables.h"
#include "tables/tableitem.h"

namespace Dxf {

SectionTABLES::SectionTABLES(File* file, QVector<CodeData>&& data)
    : SectionParser(std::move(data), file)
    , layers(file->layers())
{
}

void SectionTABLES::parse()
{
    CodeData code;
    code = nextCode();
    code = nextCode();
    while (code != "ENDSEC") {
        // Прочитать другую пару код / значение
        code = nextCode();
        if (code == "TABLE") {
            tables.resize(tables.size() + 1);
            code = nextCode();
            do {
                switch (TableItem::toType(code)) {
                case TableItem::APPID:
                    tables.last().append(new APPID(this));
                    tables.last().last()->parse(code);
                    break;
                case TableItem::BLOCK_RECORD:
                    tables.last().append(new BLOCK_RECORD(this));
                    tables.last().last()->parse(code);
                    break;
                case TableItem::DIMSTYLE:
                    tables.last().append(new DIMSTYLE(this));
                    tables.last().last()->parse(code);
                    break;
                case TableItem::LAYER:
                    LAYER* l;
                    tables.last().append(l = new LAYER(this));
                    tables.last().last()->parse(code);
                    layers[l->name] = l;
                    continue;
                case TableItem::LTYPE:
                    tables.last().append(new LTYPE(this));
                    tables.last().last()->parse(code);
                    break;
                case TableItem::STYLE:
                    tables.last().append(new STYLE(this));
                    tables.last().last()->parse(code);
                    break;
                case TableItem::UCS:
                    tables.last().append(new UCS(this));
                    tables.last().last()->parse(code);
                    break;
                case TableItem::VIEW:
                    tables.last().append(new VIEW(this));
                    tables.last().last()->parse(code);
                    break;
                case TableItem::VPORT:
                    tables.last().append(new VPORT(this));
                    tables.last().last()->parse(code);
                    break;
                }
            } while (code != "ENDTAB");
        }
    }
}
}
