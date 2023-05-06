/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "dxf_abstracttable.h"
#include "dxf_file.h"
#include "dxf_types.h"
#include "entities/dxf_graphicobject.h"

#include "datastream.h"

class Gi::Group;

namespace Dxf {

class Layer : public AbstractTable {
    friend class File;
    friend QDataStream& operator<<(QDataStream& stream, const Layer& l) {
        stream << l.groupedPaths_;
        stream << l.colorNorm_;
        stream << l.colorPath_;
        stream << l.name_;
        stream << l.lineTypeName;
        stream << l.colorNumber_;
        stream << l.flags;
        stream << l.lineWeightEnum;
        stream << l.plottingFlag;
        stream << l.graphicObjects_;
        stream << l.itemsType_;
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, Layer& l) {
        stream >> l.groupedPaths_;
        stream >> l.colorNorm_;
        stream >> l.colorPath_;
        stream >> l.name_;
        stream >> l.lineTypeName;
        stream >> l.colorNumber_;
        stream >> l.flags;
        stream >> l.lineWeightEnum;
        stream >> l.plottingFlag;
        stream >> l.graphicObjects_;
        stream >> l.itemsType_;
        return stream;
    }

public:
    Layer(File* sp);
    Layer(SectionParser* sp);
    Layer(SectionParser* sp, const QString& name);
    ~Layer() = default;
    // TableItem interface

    void parse(CodeData& code) override;
    Type type() const override { return AbstractTable::LAYER; };

    QString name() const;

    int colorNumber() const;

    const GraphicObjects& graphicObjects() const;
    void addGraphicObject(DxfGo&& go);

    Gi::Group* itemGroup() const;
    bool isEmpty() const;

    ItemsType itemsType() const;
    void setItemsType(ItemsType itemsType);

    QColor color() const;
    void setColor(const QColor& color);

    bool isVisible() const;

    void setVisible(bool visible);

private:
    Gi::Group* itemGroupNorm = nullptr;
    Gi::Group* itemGroupPath = nullptr;
    //    File* fiGle_ = nullptr;
    Pathss groupedPaths_;

    QColor colorNorm_;
    QColor colorPath_;

    QString name_;
    QString lineTypeName;

    int16_t colorNumber_ = 0;
    int16_t flags = 0;
    int16_t lineWeightEnum = 0;
    int16_t plottingFlag = 0;

    GraphicObjects graphicObjects_;
    ItemsType itemsType_ = ItemsType::Null;
    bool visible_ = true;

    enum DataEnum {
        SubclassMarker = 100, // Маркер подкласса (AcDbLayerTableRecord)
        LayerName = 2,        // Имя слоя
        Flags = 70,           // Стандартные флаги (битовые кодовые значения):
        //        1 = слой заморожен; в противном случае слой разморожен
        //        2 = слой заморожен по умолчанию на новых видовых экранах
        //        4 = слой заблокирован
        //        16 = если задано это значение, запись таблицы внешне зависима от внешней ссылки
        //        32 = если заданы и этот бит, и бит 16, внешне зависимая внешняя ссылка успешно разрешается
        //        64 = если задано это значение, то в тот момент, когда чертеж редактировался в последний раз, на запись таблицы ссылался хотя бы один объект на чертеже. (Этот флаг нужен для команд AutoCAD. Его можно игнорировать в большинстве программ для чтения файлов DXF и не нужно задавать в программах, записывающих файлы DXF)
        ColorNumber = 62,      // Номер цвета (если значение отрицательное, слой отключен)
        LineTypeName = 6,      // Имя типа линий
        PlottingFlag = 290,    // Флаг печати. Если задано значение 0, этот слой не выводится на печать
        LineWeightEnum = 370,  // Значение перечня веса линий
        PlotStyleNameID = 390, // Идентификатор/дескриптор жесткого указателя на объект PlotStyleName
        MaterialID = 347,      // Идентификатор/дескриптор жесткого указателя на объект материала
    };
};

} // namespace Dxf
