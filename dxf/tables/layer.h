#pragma once

#include "entities/dxf_graphicobject.h"
#include "tableitem.h"

namespace Dxf {

struct LAYER : TableItem {
public:
    LAYER(SectionParser* sp);
    // TableItem interface
public:
    void parse(CodeData& code) override;
    Type type() const override { return TableItem::LAYER; };

    enum VarType {
        SubclassMarker = 100, // Маркер подкласса (AcDbLayerTableRecord)
        LayerName = 2, // Имя слоя
        Flags = 70, // Стандартные флаги (битовые кодовые значения):
        //        1 = слой заморожен; в противном случае слой разморожен
        //        2 = слой заморожен по умолчанию на новых видовых экранах
        //        4 = слой заблокирован
        //        16 = если задано это значение, запись таблицы внешне зависима от внешней ссылки
        //        32 = если заданы и этот бит, и бит 16, внешне зависимая внешняя ссылка успешно разрешается
        //        64 = если задано это значение, то в тот момент, когда чертеж редактировался в последний раз, на запись таблицы ссылался хотя бы один объект на чертеже. (Этот флаг нужен для команд AutoCAD. Его можно игнорировать в большинстве программ для чтения файлов DXF и не нужно задавать в программах, записывающих файлы DXF)
        ColorNumber = 62, // Номер цвета (если значение отрицательное, слой отключен)
        LineTypeName = 6, // Имя типа линий
        PlottingFlag = 290, // Флаг печати. Если задано значение 0, этот слой не выводится на печать
        LineWeightEnum = 370, // Значение перечня веса линий
        PlotStyleNameID = 390, // Идентификатор/дескриптор жесткого указателя на объект PlotStyleName
        MaterialID = 347, // Идентификатор/дескриптор жесткого указателя на объект материала
    };

    QString name;
    QString lineTypeName;
    int colorNumber = 0;
    int flags = 0;
    int lineWeightEnum = 0;
    int plottingFlag = 0;
    std::vector<GraphicObject> gig;
};

using Layers = std::map<QString, LAYER*>;

}
