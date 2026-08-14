/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  ХХ ХХХ 2026                                                     *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

// Классы ядра с сигналами обязаны быть dllimport для потребителей libggcore
// на Windows: PMF-указатель на сигнал, взятый в exe/плагине без dllimport, --
// это адрес локального thunk'а импортной библиотеки, а moc внутри ggcore
// сравнивает его с настоящим адресом функции, и connect() не находит сигнал
// ("QObject::connect(...): signal not found"). Проверено на MinGW UCRT64
// GCC 16 под Wine: без dllimport connect через границу DLL возвращает
// невалидный QMetaObject::Connection, с ним -- работает. На ELF проблемы нет:
// адрес функции канонизируется через GOT и одинаков во всех модулях.
//
// При сборке самого ядра (GGCORE_BUILD, ставится в ggcore/CMakeLists.txt)
// макрос пуст: экспортную таблицу строит auto-export mingw-ld, а любой явный
// dllexport его бы выключил.
#if defined(_WIN32) && !defined(GGCORE_BUILD)
    #define GGCORE_EXPORT __declspec(dllimport)
#else
    #define GGCORE_EXPORT
#endif
