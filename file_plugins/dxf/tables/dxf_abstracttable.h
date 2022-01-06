/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
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

    virtual void parse(CodeData& code);

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

    //Общие групповые коды записей таблицы обозначений (DXF)

    enum CommonCodes {
        EntityName = -1, // 	Приложение: имя объекта (изменяется при каждом открытии чертежа)
        EntityType = 0, // 	Тип объекта (имя таблицы)
        HandleAllExcept = 5, // 	Дескриптор (все, кроме DIMSTYLE)
        HandleTableOnly = 105, // 	Дескриптор (только таблица DIMSTYLE)
        StartOfApplicationDefinedGroup = 102, // 	Начало определенной приложением группы "{имя_приложения". Например, "{ACAD_REACTORS" обозначает начало группы постоянных реакторов AutoCAD (необязательно)
        //CodesAndValues	=	коды, определенные приложением	, // 	Коды и значения в пределах групп с кодом 102 определяются приложением (необязательно)
        EndOfGroup = 102, // 	Конец группы, "}" (необязательно)
        IndicatesTheStartOfTheAutocadPersistentReactorsGroup = 102, // 	"{ACAD_REACTORS" обозначает начало группы постоянных реакторов AutoCAD. Эта группа присутствует, только если постоянные реакторы были присоединены к данному объекту (необязательно)
        HandleToOwnerDictionary = 330, // 	Идентификатор/дескриптор символьного указателя на словарь владельца (необязательно)
        IndicatesEndOfGroup = 102, // 	Конец группы, "}" (необязательно)
        StartOfAnExtensionDictionaryGrou = 102, // 	"{ACAD_XDICTIONARY" обозначает начало группы словаря расширений. Эта группа присутствует, только если к данному объекту были присоединены постоянные реакторы (необязательно)
        Hard_OwnerID_HandleToOwnerDictionary = 360, // 	Идентификатор/дескриптор жесткой ссылки-владельца для владельца словаря (необязательно)
        DictionaryEndOfGroup = 102, // 	Конец группы, "}" (необязательно)
        SoftPointerID_HandleToOwnerObject = 330, // 	Идентификатор/дескриптор символьного указателя на объект владельца
        SubclassMarker = 100, // 	Маркер подкласса (AcDbSymbolTableRecord)
    };
};

}
