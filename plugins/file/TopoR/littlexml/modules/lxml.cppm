// Copyright (c) 2025-2025 Damir Bakiev
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// To compile manually use a command like the folowing:
// clang++ -I ../include -std=c++20 --precompile -x c++-module lxml.cppm

module;

// #include <version>
// #include <cstddef>
// #include <cstdint>

#ifdef LXML_USE_STD_MODULE
import std;
#else
#include <cassert>
#include <memory>
#include <print>
#include <ranges>
#include <vector>
#endif

#define LXML_INTERFACE_UNIT

export module lxml;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <lxml.hpp>

