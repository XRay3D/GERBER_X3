/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "dxf_codedata.h"
#include "dxf_graphicobject.h"
#include "section/dxf_sectionparser.h"
#include <QLineF>
#include <QObject>
#include <QPolygonF>
#include <qmath.h>
#include <tuple>

#ifndef ENTITY_H
#define ENTITY_H

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QMatrix>
#else
#include <QTransform>
#endif

namespace Dxf {

QPointF polar(QPointF p, float angle /*radians*/, float distance);
double angle(QPointF p1, QPointF p2);
double signedBulgeRadius(QPointF start_point, QPointF end_point, double bulge);
std::tuple<QPointF, double, double, double> bulgeToArc(QPointF start_point, QPointF end_point, float bulge);

struct InsertEntity;

class File;

struct Entity {
    Q_GADGET
public:
    enum Type {
        NULL_ENT = -1,
        ACAD_PROXY_ENTITY,
        ARC,
        ATTDEF,
        ATTRIB,
        BODY,
        CIRCLE,
        DIMENSION,
        ELLIPSE,
        HATCH,
        HELIX,
        IMAGE,
        INSERT,
        LEADER,
        LIGHT,
        LINE,
        LWPOLYLINE,
        MESH,
        MLEADER,
        MLEADERSTYLE,
        MLINE,
        MTEXT,
        OLE2FRAME,
        OLEFRAME,
        POINT,
        POLYLINE,
        RAY,
        REGION,
        SECTION,
        SEQEND,
        SHAPE,
        SOLID,
        SPLINE,
        SUN,
        SURFACE,
        TABLE,
        TEXT,
        TOLERANCE,
        TRACE,
        UNDERLAY,
        VERTEX,
        VIEWPORT,
        WIPEOUT,
        XLINE,
        //3DFACE,
        //3DSOLID,
    };
    Q_ENUM(Type)
    static Type toType(const QString& key);

    Codes data;
    SectionParser* sp = nullptr;
    Entity(SectionParser* sp);
    virtual ~Entity() { }
    virtual void draw(const InsertEntity* const i = nullptr) const;

    virtual void parse(CodeData& code);
    virtual Type type() const = 0;
    virtual GraphicObject toGo() const = 0;

    static QString typeName(int key);
    QString name() const;
    QColor color() const;
    void attachToLayer(GraphicObject&& go) const;

    QString layerName;
    QString handle;
    QString softPointerID;
    int16_t colorNumber = 0;

#ifdef QT_DEBUG
    static constexpr double u = 10.;
    static constexpr double d = 0.1;
#else
    static constexpr double u = 100.;
    static constexpr double d = 0.01;
#endif
    enum DataEnum {
        EntityName = -1, //    Приложение: имя объекта (изменяется при каждом открытии чертежа)
        //    не пропускается
        EntityType = 0, //    Тип объекта
        //    не пропускается
        Handle = 5, //    Дескриптор
        //    не пропускается

        //        E102 = 102,
        //        //    Начало определенной в приложении группы
        //        //    "{имя_приложения" (необязательно)
        //        //    без значения по умолчанию
        //        //    коды, определенные в приложении
        //        //    Коды и значения в группах с кодом 102 определяются в приложении (необязательно)
        //        //    без значения по умолчанию
        //        E102 = 102,
        //        //    Конец группы, "}" (необязательно)
        //        //    без значения по умолчанию
        //        E102 = 102,
        //        //    "{ACAD_REACTORS" обозначает начало группы постоянных реакторов AutoCAD. Эта группа присутствует, только если постоянные реакторы были присоединены к данному объекту (необязательно)
        //        //    без значения по умолчанию

        SoftPointerID = 330, //    Идентификатор/дескриптор символьного указателя на словарь владельца (необязательно)
        //    без значения по умолчанию

        //        E102 = 102,        //    Конец группы, "}" (необязательно)
        //        //    без значения по умолчанию
        //        E102 = 102,        //    "{ACAD_XDICTIONARY" обозначает начало группы словаря расширения. Эта группа присутствует, только если словарь расширения был прикреплен к объекту (необязательно)
        //        //    без значения по умолчанию

        HardOwnerID = 360, //    Идентификатор/дескриптор жесткого указателя на словарь владельца (необязательно)
        //    без значения по умолчанию
        //        E102 = 102,
        //        //    Конец группы, "}" (необязательно)
        //    без значения по умолчанию
        //        E330 = 330,        //    Идентификатор/дескриптор символьного указателя на объект BLOCK_RECORD владельца
        //        //    не пропускается

        SubclassMarker = 100, //    Маркер подкласса (AcDbEntity)
        //    не пропускается
        E67 = 67, //    Отсутствие значения или ноль указывают на наличие объекта в пространстве модели. 1 указывает на наличие объекта в пространстве листа (необязательно).
        //E0 = 0,
        E410 = 410,
        //    Приложение: имя вкладки листа
        //    не пропускается
        LayerName = 8, //    Имя слоя
        //    не пропускается
        LineType = 6, //    Имя типа линий (присутствует, если значение — не ПОСЛОЮ). Специальное имя ПОБЛОКУ указывает на плавающий тип линий (необязательно)
        //    ПОСЛОЮ
        E347 = 347, //    Идентификатор/дескриптор жесткого указателя объекта материала (присутствует, если значение — не ПОСЛОЮ)
        //    ПОСЛОЮ
        ColorNumber = 62, //    Номер цвета (присутствует, если значение — не ПОСЛОЮ); ноль указывает на цвет ПОБЛОКУ (плавающий); 256 указывает на цвет ПОСЛОЮ; отрицательное значение указывает на то, что слой отключен (необязательно)
        //    ПОСЛОЮ
        LineWeight = 370, //    Значение перечня веса линий. Сохраняется и перемещается как 16-разрядное целое число.
        //    не пропускается
        LineTypeScale = 48, //    Масштаб типа линий (необязательно)
        //    1.0
        Visibility = 60, //    Видимость объекта (необязательно):
        //    0 = видимые
        //    1 = невидимые
        //    0,
        NumberOfBytes = 92, //    Количество байтов в графике объекта прокси, представленной в последующих группах с кодом 310, которые являются записями двоичного уровня (необязательно)
        //    без значения по умолчанию
        BinaryChunk = 310, //    Графика объекта прокси (несколько строк; макс. 256 символов в каждой строке) (необязательно)
        //    без значения по умолчанию
        A24bitColor = 420, //    Цветовое 24-битное значение, рассматриваемое в контексте байтов со значением от 0 до 255. Младшим байтом является синее значение, средним байтом — зеленое, а третьему байту соответствует красное значение. Высшим байтом всегда является 0. Групповой код нельзя использовать в пользовательских объектах для собственных данных, так как групповой код зарезервирован для цветовых данных уровня класса AcDbEntity и данных прозрачности уровня класса AcDbEntity
        //    без значения по умолчанию
        ColorName = 430, //    Имя цвета. Групповой код нельзя использовать в пользовательских объектах для собственных данных, так как групповой код зарезервирован для цветовых данных уровня класса AcDbEntity и данных прозрачности уровня класса AcDbEntity
        //    без значения по умолчанию
        TransparencyValue = 440, //    Значение прозрачности. Групповой код нельзя использовать в пользовательских объектах для собственных данных, так как групповой код зарезервирован для цветовых данных уровня класса AcDbEntity и данных прозрачности уровня класса AcDbEntity
        //    без значения по умолчанию
        PlotStyleID = 390, //    Идентификатор/дескриптор объекта стиля печати
        //    без значения по умолчанию
        ShadowMode = 284, //    Режим теней
        //    0 = отображение отбрасываемой и падающей теней
        //    1 = отображение отбрасываемой тени
        //    2 = отображение падающей тени
        //    3 = игнорирование теней
        //    Прим.: Это свойство является устаревшим в программных продуктах на базе AutoCAD начиная с версии 2016. Тем не менее оно поддерживается для обеспечения обратной совместимости с предыдущими версиями.
        //    без значения по умолчанию
    };
    Q_ENUM(DataEnum)
    static DataEnum toDataEnum(const QString& key);

    enum Points {
        PrimaryX = 10,
        Other1X = 11,
        Other2X = 12,
        Other3X = 13,
        Other4X = 14,
        Other5X = 15,
        Other6X = 16,
        Other7X = 17,
        Other8X = 18,

        PrimaryY = 20,
        Other1Y = 21,
        Other2Y = 22,
        Other3Y = 23,
        Other4Y = 24,
        Other5Y = 25,
        Other6Y = 26,
        Other7Y = 27,
        Other8Y = 28,

        PrimaryZ = 30,
        Other1Z = 31,
        Other2Z = 32,
        Other3Z = 33,
        Other4Z = 34,
        Other5Z = 35,
        Other6Z = 36,
        Other7Z = 37,
        Other8Z = 38,

    };
};
}
#endif // ENTITY_H

#include "dxf_arc.h"
#include "dxf_attdef.h"
#include "dxf_circle.h"
#include "dxf_dummy.h"
#include "dxf_ellipse.h"
#include "dxf_hatch.h"
#include "dxf_insert.h"
#include "dxf_line.h"
#include "dxf_lwpolyline.h"
#include "dxf_mtext.h"
#include "dxf_point.h"
#include "dxf_polyline.h"
#include "dxf_solid.h"
#include "dxf_spline.h"
#include "dxf_text.h"
//#include "dxf_attrib.h"
//#include "dxf_body.h"
//#include "dxf_dimension.h"
//#include "dxf_helix.h"
//#include "dxf_image.h"
//#include "dxf_leader.h"
//#include "dxf_light.h"
//#include "dxf_mesh.h"
//#include "dxf_mleader.h"
//#include "dxf_mleaderstyle.h"
//#include "dxf_mline.h"
//#include "dxf_ole2frame.h"
//#include "dxf_oleframe.h"
//#include "dxf_ray.h"
//#include "dxf_region.h"
//#include "dxf_section.h"
//#include "dxf_shape.h"
//#include "dxf_sun.h"
//#include "dxf_surface.h"
//#include "dxf_table.h"
//#include "dxf_tolerance.h"
//#include "dxf_trace.h"
//#include "dxf_underlay.h"
//#include "dxf_vertex.h"
//#include "dxf_viewport.h"
//#include "dxf_wipeout.h"
//#include "dxf_xline.h"
