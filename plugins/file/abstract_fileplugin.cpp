/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "abstract_fileplugin.h"

#include "geo/cancel.h"

// Отмена разбора устроена так же, как отмена вычислений в Geo: областной
// стоп-токен плюс кооперативные проверки внутри парсера. Своего механизма
// заводить незачем -- этот уже есть, покрыт тестами и умеет прерывать в том
// числе тяжёлые операции над регионами, в которые парсер заходит на финальном
// mergedCurves()/groupedPaths().
//
// Ключ реестра -- ПОЛНЫЙ путь файла: он же ключ строки в окне прогресса.

void AbstractFilePlugin::requestCancel(const QString& fileName) {
    std::lock_guard lock{tasksMutex_};
    // emplace, а не find: файл может ещё стоять в очереди parserThread и не
    // дойти до parseFileTask. Тогда токен ждёт его здесь, и слот, добравшись до
    // очереди, сразу увидит запрос и выйдет, не начав разбор.
    tasks_[fileName].request_stop();
}

void AbstractFilePlugin::parseFileTask(const QString& fileName, uint32_t type_) {
    if(type_ != type()) return; // не наш файл -- его разберёт другой плагин

    std::stop_token token;
    {
        std::lock_guard lock{tasksMutex_};
        token = tasks_[fileName].get_token();
    }

    auto forget = [this, &fileName] {
        std::lock_guard lock{tasksMutex_};
        tasks_.erase(fileName);
    };

    if(token.stop_requested()) { // отменили, пока стоял в очереди
        forget();
        emit fileCanceled(fileName);
        emit fileProgress(fileName, 1, 1);
        return;
    }

    Geo::CancelScope scope{token};
    try {
        parseFile(fileName, type_);
    } catch(const Geo::Cancelled&) {
        // Штатный выход по кнопке отмены, а не ошибка: в лог не пишем.
        // Недостроенный файл парсер удаляет сам, в своём catch.
        emit fileCanceled(fileName);
        emit fileProgress(fileName, 1, 1);
    }
    forget();
}
