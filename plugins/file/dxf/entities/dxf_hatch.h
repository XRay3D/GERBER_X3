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

#include "dxf_entity.h"

namespace Dxf {

// штриховка

struct Hatch final : Entity {
    Hatch(SectionParser* sp);
    ~Hatch();
    // Entity interface
    // void draw(const InsertEntity* const i = nullptr) const override;

    void parse(CodeData& code) override;
    Type type() const override;
    ;
    GraphicObject toGo() const override;
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

    enum DataEnum {
        SubclassMarker = 100, // 	Маркер подкласса (AcDbHatch)
        ElevationPointX = 10, // 	Точка отметки (в ОСК)  	Файл DXF: значение X = 0; приложение: 3D-точка (X и Y всегда равны 0, Z представляет значение отметки)
        ElevationPointY = 20, // 	Файл DXF: значения Y и Z для точки отметки (в ОСК)
        ElevationPointZ = 30, // 	Значение Y = 0, Z представляет значение отметки
        ExtrDirectionX = 210, // 	Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)  	Файл DXF: значение X; приложение: 3D-вектор
        ExtrDirectionY = 220, // 	Файл DXF: значения Y и Z направления выдавливания
        ExtrDirectionZ = 230,
        HatchPatternName = 2,                // 	Имя образца штриховки
        SolidFillFlag = 70,                  // 	Флаг сплошной заливки (0 = заливка штриховкой, 1 = сплошная заливка); для мполигона — версия мполигона
        PatternFillColor = 63,               // 	Для мполигона — цвет заливки штриховкой (как ACI)
        /*MPolygon*/ AssociativityFlag = 71, // 	Флаг ассоциативности (0 = неассоциативный, 1 = ассоциативный); для мполигона — флаг сплошной заливки (0 = без сплошной заливки; 1 = со сплошной заливкой)
        NumberOfBoundaryPaths = 91,          // 	Число траекторий контуров (замкнутых контуров)
        //	=	Различается	, // 	Данные траекторий контуров. Повторяется столько раз, сколько задано кодом 91. См. "Данные траекторий контуров"
        HatchStyle = 75, // 	Стиль штриховки:
        //	0 = Hatch “odd parity” area 	=		, // 	0 = область штриховки по принципу "кратность двум" (обычный стиль)
        //	1 = Hatch outermost area only 	=		, // 	1 = штриховка только крайней области (внешний стиль)
        //	2 = Hatch through entire area 	=		, // 	2 = штриховка по всей площади (игнорирование стиля)
        HatchPatternType = 76, // 	Тип образца штриховки:
        //	0 = User-defined	=		, // 	0 = из линий
        //	1 = Predefined	=		, // 	1 = стандартный
        //	2 = Custom	=		, // 	2 = пользовательский
        HatchPatternAngle = 52,                   // 	Угол образца штриховки (только для заливки штриховкой)
        HatchPatternScaleOrSpacing = 41,          // 	Масштаб или интервал образца штриховки (только для заливки штриховкой)
        /*MPolygon*/ BoundaryAnnotationFlag = 73, // 	Для мполигона — флаг аннотации контура:
        //	0 = boundary is not an annotated boundary	=		, // 	0 = контур не является аннотированным
        //	1 = boundary is an annotated boundary	=		, // 	1 = контур представляет собой аннотированный контур
        HatchPatternDoubleFlag = 77, // 	Флаг удвоения образца штриховки (только для заливки штриховкой):
        //	0 = not double	=		, // 	0 = не двойной
        //	1 = double	=		, // 	1 = двойной
        NumberOfPatternDefinitionLines = 78, // 	Число линий определения образца
        //		=	Различается	, // 	Данные линий образца. Повторяется столько раз, сколько задано кодом 78. См. "Данные образца"
        PixelSize = 47,                                    // 	Размер в пикселях, используемый для определения плотности при выполнении различных операций пересечения и отбрасывания лучей в процессе расчета образца штриховки для ассоциативных штриховок и штриховок, созданных с помощью метода затопления
        NumberOfSeedPoints = 98,                           // 	Количество точек-прототипов
        /*MPolygon*/ OffsetVector = 11,                    // 	Для мполигона — вектор смещения
        /*MPolygon*/ NumberOfDegenerateBoundaryPaths = 99, // 	Для мполигона — количество вырожденных траекторий контуров (замкнутых контуров), где вырожденная траектория контура является границей, игнорируемой штриховкой
        SeedPointX = 10,                                   // 	Точка-прототип (в ОСК)
        // 	Файл DXF: значение X; приложение: 2D-точка (несколько записей)
        SeedPointY = 20,                     // 	Файл DXF: значение Y точки-прототипа (в ОСК); (несколько записей)
        IndicatesSolidHatchOrGradient = 450, // 	Указание сплошной штриховки или градиента; если штриховка сплошная, значения для оставшихся кодов игнорируются, но должны присутствовать. Необязательно; если код 450 указан в файле, то также в файле должны быть и следующие коды: 451, 452, 453, 460, 461, 462 и 470. Если кода 450 нет в файле, то в файле не должно быть следующих кодов: 451, 452, 453, 460, 461, 462 и 470
        // 	0 = Solid hatch	=		, // 	0 = сплошная штриховка
        //	1 = Gradient	=		, // 	1 = градиент
        Zero /*is reserved for future use*/ = 451,                                                // 	Ноль зарезервирован для последующего использования
        RecordsColors /*Records how colors were defined and is used only by dialog code:*/ = 452, // 	Запись способа определения цветов. Используется только кодом диалогового окна:
        //	0 = Two-color gradient	=		, // 	0 = двухцветный градиент
        //	1 = Single-color gradient	=		, // 	1 = одноцветный градиент
        NumberOfColors = 453, // 	Количество цветов:
        //	0 = Solid hatch	=		, // 	0 = сплошная штриховка
        //  2 = Gradient =, // 	2 = градиент
        RotationAangleInRadiansForGradients = 460, // 	Угол поворота в радианах для градиента (по умолчанию = 0, 0)
        GradientDefinition = 461,                  // 	Определение градиента; соответствует параметру "По центру" на вкладке "Градиент" диалогового окна "Штриховка и заливка контура". Каждый градиент имеет два определения: со сдвигом и без. Значение сдвига описывает сглаживание двух определений, которые должны использоваться. Значение 0,0 означает, что следует использовать только версию без сдвига, а значение 1,0 означает, что следует использовать только версию со сдвигом.
        ColorTintValueUsedByDialogCode = 462,      // 	Значение оттенка цвета, используемое в коде диалогового окна (по умолчанию = 0, 0; диапазон — от 0,0 до 1,0). Значение оттенка цвета представляет собой цвет градиента и определяет степень оттенка в диалоговом окне, если для группового кода штриховки 452 установлено значение 1.
        ReservedForFutureUse = 463,                // 	Зарезервировано для дальнейшего использования:
        //	0 = First value	=		, // 	0 = первое значение
        //	1 = Second value	=		, // 	1 = второе значение
        String = 470, // 	Строка (по умолчанию = LINEAR)
    };

    // Данные штриховки (DXF)
    enum PatternData {
        Angle = 53,      // Угол линий штриховки
        BaseX = 43,      // Базовая точка линий штриховки, компонент X
        BaseY = 44,      // Базовая точка линий штриховки, компонент Y
        Offset = 45,     // Смещение линии штриховки, компонент X
        OffsetY = 46,    // Смещение линии штриховки, компонент Y
        DashCoount = 79, // Количество элементов длины штриха
        DashLength = 49, // Длина штриха (несколько записей)
    };
    // Данные траекторий контуров (DXF)

    // Групповые коды данных траектории контура штриховки
    enum HatchDataCodes {
        PathTypeFlag = 92, // Флаг типа траектории контура (битовый код):
        // 0 = по умолчанию
        // 1 = внешняя
        // 2 = полилиния
        // 4 = производная
        // 8 = текстовое поле
        // 16 = крайняя
        //  Различается // Данные типа контура полилинии (только если контур = полилиния). См. таблицу данных контура полилинии ниже
        NumberOfEdges = 93, // Количество кромок в этой траектории контура (только если контур не является полилинией)
        EdgeType = 72,      // Тип кромки (только если контур не является полилинией):
        // 1 = линейная
        // 2 = дуга окружности
        // 3 = дуга эллипса
        // 4 = сплайн
        //  Различается // Данные типа кромки (только если контур не является полилинией). См. соответствующую таблицу данных кромок ниже
        NumberOfSourceBoundaryObjects = 97,     // Количество исходных объектов контура
        ReferenceToSourceBoundaryObjects = 330, // Ссылка на исходные объекты контура (несколько записей)
    };
    // Флаг типа траектории контура
    enum PathTypeFlags {
        Default = 0,    // по умолчанию
        External = 1,   // внешняя
        Polyline = 2,   // полилиния
        Derived = 4,    // производная
        Textbox = 8,    // текстовое поле
        Outermost = 16, // крайняя
    };
    // Тип кромки
    enum EdgeTypes {
        Line = 1,        // линейная
        CircularArc = 2, // дуга окружности
        EllipticArc = 3, // дуга эллипса
        Spline = 4,      // сплайн
    };
    // Групповые коды данных контура полилинии
    enum PolylineDataCodes {
        HasBulgeFlag = 72,             // Флаг наличия прогиба
        IsClosedFlag = 73,             // Флаг замкнутости
        NumberOfPolylineVertices = 93, // Число вершин полилинии
        VertexLocationX = 10,          // Местоположение вершины (в ОСК)        Файл DXF: значение X; приложение: 2D-точка (несколько записей)
        VertexLocationY = 20,          // Файл DXF: значение Y местоположения вершины (в ОСК) (несколько записей)
        Bulge = 42,                    // Прогиб (необязательно, по умолчанию = 0)
    };
    // Групповые коды данных линейной кромки
    enum LineEdgeDataCodes {
        StartPointX = 10, // Начальная точка (в ОСК)        Файл DXF: значение X; приложение: 2D-точка
        StartPointY = 20, // Файл DXF: значение Y начальной точки (в ОСК)
        EndPointX = 11,   // Конечная точка (в ОСК)        Файл DXF: значение X; приложение: 2D-точка
        EndPointY = 21,   // Файл DXF: значение Y конечной точки (в ОСК)
    };
    // Групповые коды данных дуговой кромки
    enum ArcEdgeDataCodes {
        CenterPointX = 10,           // Центральная точка (в ОСК)    Файл DXF: значение X; приложение: 2D-точка
        CenterPointY = 20,           // Файл DXF: значение Y центральной точки (в ОСК)
        Radius = 40,                 // Радиус
        StartAngle = 50,             // Начальный угол
        EndAngle = 51,               // Конечный угол
        IsCounterClockwiseFlag = 73, // Флаг расположения против часовой стрелки
    };

    // Групповые коды данных эллиптической кромки
    enum EllipseEdgeDataCodes {
        // CenterPointX = 10, // Центральная точка (в ОСК)  Файл DXF: значение X; приложение: 2D-точка
        // CenterPointY = 20, // Файл DXF: значение Y центральной точки (в ОСК)
        EndpointQfMajorAxisRelativeToCenterPointX = 11, // Конечная точка большой оси относительно центральной точки (в ОСК)        Файл DXF: значение X; приложение: 2D-точка
        EndpointQfMajorAxisRelativeToCenterPointY = 21, // Файл DXF: значение Y конечной точки большой оси (в ОСК)
        LengthOfMinorAxis = 40,                         // Длина малой оси (в процентах от длины большой оси)
        // StartAngle = 50, // Начальный угол
        // EndAngle = 51, // Конечный угол
        // IsCounterClockwiseFlag = 73, // Флаг расположения против часовой стрелки
    };
    // Групповые коды данных сплайновой кромки
    enum SplineEdgeDataCodes {
        Degree = 94,                // Порядок
        Rational = 73,              // Рациональный
        Periodic = 74,              // Периодический
        NumberOfKnots = 95,         // Число узлов
        NumberOfControlPoints = 96, // Число управляющих точек
        KnotValues = 40,            // Значения узлов (несколько записей)
        ControlPointX = 10,         // Управляющая точка (в ОСК)        Файл DXF: значение X; приложение: 2D-точка
        ControlPointY = 20,         // Файл DXF: значение Y управляющей точки (в ОСК)
        Weights = 42,               // Веса (не обязательно, по умолчанию = 1)
        NumberOfFitData = 97,       // Число определяющих данных
        FitDatumX = 11,             // Определяющая база отсчета (в ОСК)        Файл DXF: значение X; приложение: 2D-точка
        FitDatumY = 21,             // Файл DXF: значение Y определяющей базы отсчета (в ОСК)
        StartTangentX = 12,         // Начальная касательная        Файл DXF: значение X; приложение: 2D-вектор
        StartTangentY = 22,         // Файл DXF: значение Y начальной касательной (в ОСК)
        EndTangentX = 13,           // Конечная касательная        Файл DXF: значение X; приложение: 2D-вектор
        EndTangentY = 23,           // DXF: значение Y конечной касательной (в ОСК)
    };

    struct Edge {
        Edge(int type)
            : type(type) {
        }
        virtual ~Edge() = default;
        virtual QPolygonF toPolygon() const = 0;
        const int type;
    };
    struct LineEdge : Edge {
        LineEdge(int type)
            : Edge(type) {
        }
        QPointF p1;
        QPointF p2;
        QPolygonF toPolygon() const override {
            QPolygonF p(2);
            p[0] = p1;
            p[1] = p2;
            return p;
        }
    };
    struct CircularArcEdge : Edge {
        CircularArcEdge(int type)
            : Edge(type) {
        }
        QPolygonF toPolygon() const override {
            QPolygonF p;
            return p;
        }
    };
    struct EllipticArcEdge : Edge {
        EllipticArcEdge(int type)
            : Edge(type) {
        }
        QPolygonF toPolygon() const override {
            QPolygonF p;
            return p;
        }
    };
    struct SplineEdge : Edge {
        SplineEdge(int type)
            : Edge(type) {
        }
        QPolygonF toPolygon() const override {
            QPolygonF p;
            return p;
        }
    };

    mvector<mvector<Edge*>> edges;

    mvector<QString> referencesToSourceBoundaryObject; // Ссылка на исходные объекты контура (несколько записей)

    QPointF centerPoint;
    std::vector<int> pathTypeFlags;
    int16_t edgeType = 0;
    double thickness = 0;
    double radius = 0;
};

} // namespace Dxf
