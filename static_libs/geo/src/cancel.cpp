#include "geo/cancel.h"

#include <utility>

namespace Geo {

namespace {

// Действующая область отмены. Именно thread_local, а не общая на процесс:
// параллельных вычислений может идти несколько (в приложении с вкладками
// -- по одному на вкладку), и отмена одного не должна ронять остальные.
// Рабочие потоки Geo заносят сюда токен своего заказчика сами.
thread_local std::stop_token current;

} // namespace

const char* Cancelled::what() const noexcept {
    return "Geo: вычисление прервано по запросу отмены";
}

CancelScope::CancelScope(std::stop_token token) noexcept
    : previous_{std::exchange(current, std::move(token))} { }

CancelScope::~CancelScope() {
    current = std::move(previous_);
}

std::stop_token cancelToken() noexcept {
    return current;
}

bool cancelRequested() noexcept {
    return current.stop_requested();
}

void checkCancelled() {
    if(current.stop_requested()) throw Cancelled{};
}

} // namespace Geo
