#include "layer.h"

#include <QGraphicsScene>


namespace Dxf {
LAYER::LAYER(SectionParser* sp)
    : TableItem(sp)
{
    //scene->addItem(gig = new QGraphicsItemGroup);
}

void LAYER::parse(CodeData& code)
{
    do {
        data << code;
        switch (code.code()) {
        case SubclassMarker:
            break;
        case LayerName:
            name = code;
            break;
        case Flags:
            flags = code;
            break;
        case ColorNumber:
            colorNumber = code;
            break;
        case LineTypeName:
            lineTypeName = code;
            break;
        case PlottingFlag:
            plottingFlag = code;
            break;
        case LineWeightEnum:
            lineWeightEnum = code;
            break;
        case PlotStyleNameID:
            break;
        case MaterialID:
            break;
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}
}
