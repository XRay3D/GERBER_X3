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
#include "dxf_tableitem.h"
namespace Dxf {
struct AppId final : TableItem {
public:
    AppId(SectionParser* sp);

    enum VarType {
        SubclassMarker = 100, // Маркер подкласса (AcDbRegAppTableRecord)
        ApplicationName = 2, // Имя приложения, созданное пользователем или приложением (для расширенных данных). В этих записях таблицы содержится набор имен всех зарегистрированных приложений
        StandardFlag = 70, // Стандартные значения флагов (кодовые битовые значения):
        //        16 = если задано это значение, запись таблицы внешне зависима от внешней ссылки
        //        32 = если заданы и этот бит, и бит 16, внешне зависимая внешняя ссылка успешно разрешается
        //        64 = если задано это значение, то в тот момент, когда чертеж редактировался в последний раз, на запись таблицы ссылался хотя бы один объект на чертеже. (Этот флаг нужен для команд AutoCAD. Его можно игнорировать в большинстве программ для чтения файлов DXF и не нужно задавать в программах, записывающих файлы DXF)
    };
    // TableItem interface
public:
    void parse(CodeData& code) override;
    Type type() const override { return TableItem::APPID; };

    QString applicationName;
    int standardFlag = 0;
};
}
