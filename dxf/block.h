#pragma once

#include "codedata.h"
#include <QVector>
namespace Dxf {
struct Entity;
struct SectionParser;

struct Block {
    SectionParser* sp;
    Block(QMap<QString, Block*>& blocks ,SectionParser* sp = nullptr);
    ~Block();

    void parse(CodeData& code);
    void parseHeader(CodeData& code);
    void parseData(CodeData& code);

    enum VarType {
        EntityType = 0, // Тип объекта (BLOCK)
        Handle = 5, // Дескриптор
        StartOfApplication_definedGroup = 102, // Начало определенной приложением группы "{имя_приложения". Например, "{ACAD_REACTORS" обозначает начало группы постоянных реакторов AutoCAD (необязательно)
        // коды, определенные в приложении
        // Коды и значения в пределах группы с кодом 102,//,//определяются в приложении (необязательно)
        EndOfGroup = 102, // Конец группы, "}" (необязательно)
        SoftPointerID = 330, // Идентификатор/дескриптор символьного указателя на объект владельца
        SubclassMarker = 100, // Маркер подкласса (AcDbEntity)
        LayerName = 8, // Имя слоя
        SubclassMarker_2 = 100, // Маркер подкласса (AcDbBlockBegin)
        BlockName = 2, // Имя блока
        BlockTypeFlags = 70, // Флаги типа блока (кодовые битовые значения, могут быть скомбинированы):
        BasePointX = 10, // Базовая точка  Файл DXF: значение X; приложение: 3,//D-точка
        BasePointY = 20, //Файл DXF: значение Y и Z базовой точки
        BasePointZ = 30, //
        BlockName_2 = 3, // Имя блока
        XrefPathName = 1, // Имя пути внешней ссылки
        BlockDescription = 4, // Описание блока (необязательно)
    };

    enum BTFlags {
        BTF0 = 0, // ни один из следующих флагов не применяется
        BTF1 = 1, // это анонимный блок, созданный с помощью штриховки, нанесения ассоциативных размеров, других внутренних операций или приложения
        BTF2 = 2, // этот блок содержит непостоянные определения атрибутов (данный бит не задается, если блок содержит все определения атрибутов, которые являются постоянными, или вовсе не содержит определений атрибутов)
        BTF4 = 4, // этот блок является внешней ссылкой
        BTF8 = 8, // этот блок является наложением внешней ссылки
        BTF16 = 16, // этот блок является зависимым извне
        BTF32 = 32, // это разрешенная внешняя ссылка или ссылка, зависимая от внешней ссылки (игнорируется при вводе)
        BTF64 = 64, // это определение является связанной внешней ссылкой (игнорируется при вводе)
    };

    BTFlags flags = BTF0;
    QString blockName;
    QString layerName;
    QString blockDescription;
    QPointF basePoint;

    QVector<CodeData> bData;
    QVector<Entity*> entities;
    QMap<QString, Block*>& blocks;
};
}
