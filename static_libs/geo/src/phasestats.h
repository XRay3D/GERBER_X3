#pragma once

// Внутренняя сторона geo/phasestats.h: сами фазы и RAII-замер. Наружу не
// выходит -- расстановка замеров дело реализации, отчёт публичен.

#include "geo/phasestats.h"

#include <chrono>
#include <cstdint>

namespace Geo {

enum class Phase : std::uint8_t { Capsules, JoinAll, RegionOp, Materialize, Count_ };

void phaseAdd(Phase phase, std::uint64_t ns) noexcept;

struct PhaseScope {
    explicit PhaseScope(Phase phase)
        : phase{phase} { }
    ~PhaseScope() {
        phaseAdd(phase, std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::steady_clock::now() - start)
                            .count());
    }
    PhaseScope(const PhaseScope&) = delete;
    PhaseScope& operator=(const PhaseScope&) = delete;

private:
    Phase phase;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
};

} // namespace Geo
