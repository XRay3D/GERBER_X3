module;

#ifdef _MSVC_LANG
#pragma warning(disable : 5202)
#endif

// import std;
#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <cstddef>
// #include <cstdint>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

export module ctre;

#define CTRE_IN_A_MODULE
#define CTLL_IN_A_MODULE
#define UNICODE_DB_IN_A_MODULE

using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::int8_t;
using std::size_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;

// #include "unicode-db.hpp"
#include "ctre.hpp"
