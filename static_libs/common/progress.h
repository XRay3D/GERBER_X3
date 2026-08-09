#pragma once

// Ход долгого расчёта -- сколько шагов всего и сколько пройдено. Ровно это и
// показывает полоса прогресса, и ничего больше здесь нет.
//
// ОТМЕНЫ здесь больше нет: она целиком в Geo (geo/cancel.h). Тамошняя
// областная (CancelScope + std::stop_token) и срабатывает ВНУТРИ точных
// операций -- между контурами в parallelFor, между кусками дерева слияний, --
// а прежний флаг проверялся только там, куда его успевали поставить руками,
// то есть между вызовами. Отменить одно большое объединение регионов он не мог
// вовсе, а ждать его приходилось дольше всего.
//
// Счётчики ГЛОБАЛЬНЫЕ и атомарные: пишет их расчётный поток, читает GUI, а
// добраться до них надо и оттуда, где никакого объекта под рукой нет (jcv_*
// внутри вороного).

#include <atomic>

class Progress {
    static inline std::atomic<int> max_;
    static inline std::atomic<int> current_;

public:
    static void reset() {
        current_ = 0;
        max_ = 0;
    }

    static int max() { return max_; }
    static void setMax(int max) { max_ = max; }
    static int current() { return current_; }
    static void setCurrent(int current = 0) { current_ = current; }
    static void incCurrent() { ++current_; }
};
