#include "phasestats.h"

#include <array>
#include <atomic>
#include <cstdio>

namespace Geo {

namespace {

struct Acc {
    std::atomic<std::uint64_t> ns{};
    std::atomic<std::uint64_t> calls{};
};

constexpr std::size_t phaseCount = static_cast<std::size_t>(Phase::Count_);
std::array<Acc, phaseCount> accs;

constexpr std::array<const char*, phaseCount> names{
    "capsules", "joinAll", "regionOp", "materialize", "adopt"};

} // namespace

void phaseAdd(Phase phase, std::uint64_t ns) noexcept {
    Acc& acc = accs[static_cast<std::size_t>(phase)];
    acc.ns.fetch_add(ns, std::memory_order_relaxed);
    acc.calls.fetch_add(1, std::memory_order_relaxed);
}

std::string phaseReport(bool reset) {
    std::string report;
    for(std::size_t i = 0; i < phaseCount; ++i) {
        const std::uint64_t ns = reset ? accs[i].ns.exchange(0, std::memory_order_relaxed)
                                       : accs[i].ns.load(std::memory_order_relaxed);
        const std::uint64_t calls = reset ? accs[i].calls.exchange(0, std::memory_order_relaxed)
                                          : accs[i].calls.load(std::memory_order_relaxed);
        char line[96];
        std::snprintf(line, sizeof line, "%-12s %8llu calls %10.1f ms\n", names[i],
            static_cast<unsigned long long>(calls), ns / 1e6);
        report += line;
    }
    return report;
}

} // namespace Geo
