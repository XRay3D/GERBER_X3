/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  August 09, 2026                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

// Обход полей агрегатов через статическую рефлексию C++26 (P2996).
// Основа JSON-сериализации (common/serial.h): обход полей, их имена и счёт.

#include <meta>
#include <ranges>
#include <type_traits>

static constexpr auto CTX = std::meta::access_context::current();

template <typename T>
consteval auto fields_count() {
    return nonstatic_data_members_of(^^T, CTX).size();
}

template <typename T, typename Func>
    requires(is_class_type(^^std::remove_cvref_t<T>))
constexpr auto for_each_field(T&& str, Func&& func) {
    static constexpr auto MEMBERS = std::define_static_array(
        nonstatic_data_members_of(^^std::remove_cvref_t<T>, CTX));

    template for(constexpr auto MEMBER: MEMBERS) {
        if constexpr(
            requires {
                func(std::forward<T>(str).[:MEMBER:]);
            })
            func(std::forward<T>(str).[:MEMBER:]);
        else if constexpr(
            requires {
                func(std::forward<T>(str).[:MEMBER:], identifier_of(MEMBER));
            })
            func(std::forward<T>(str).[:MEMBER:], identifier_of(MEMBER));
        else
            static_assert("no mach func!");
    }
}

template <size_t I, typename T>
constexpr auto get_name() {
    return identifier_of(nonstatic_data_members_of(^^T, CTX)[I]);
}
