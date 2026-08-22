#pragma once

#include <string>

namespace Geo {

// Отчёт по фазам горячего пути Inflate: капсулы границы (boundaryCapsules),
// слияния joinAll, булевы операции региона в Inflate, материализация
// bulge-вида. Счётчики всегда включены -- пара атомарных сложений на вызов
// против сотен миллисекунд самих фаз; параллельные вызовы складываются, так
// что сумма фазы -- это занятость, а не стена. Печатает и обнуляет их
// geo_bench_pocket при A/B-замерах; reset = false оставляет накопленное.
std::string phaseReport(bool reset = true);

} // namespace Geo
