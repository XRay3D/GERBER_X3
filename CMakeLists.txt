#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  ХХ ХХХ 2026
 Website   :  na
 Copyright :  Damir Bakiev 2016-2026
 License   :
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

# set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
# set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

project(GERBER_X3 VERSION 0.9.9)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_UNITY_BUILD ON)

# Используем mold/lld, если они есть в системе и реально поддерживаются текущим
# тулчейном (просто наличия бинарника недостаточно: например, mold из
# Homebrew/MSYS2 может присутствовать, но не поддерживаться CMAKE_LINKER_TYPE на
# конкретной платформе/компиляторе).
include(CheckLinkerFlag)
find_program(MOLD_EXECUTABLE mold)
if(MOLD_EXECUTABLE)
  check_linker_flag(CXX "-fuse-ld=mold" CXX_LINKER_SUPPORTS_MOLD)
endif()
if(CXX_LINKER_SUPPORTS_MOLD)
  set(CMAKE_LINKER_TYPE MOLD)
else()
  find_program(LLD_EXECUTABLE NAMES ld.lld lld)
  if(LLD_EXECUTABLE)
    check_linker_flag(CXX "-fuse-ld=lld" CXX_LINKER_SUPPORTS_LLD)
  endif()
  if(CXX_LINKER_SUPPORTS_LLD)
    set(CMAKE_LINKER_TYPE LLD)
  endif()
endif()

# Проект собирают компилятором посвежее системного, и его libstdc++ требует
# версий символов (GLIBCXX_3.4.35 и далее), которых в /lib64/libstdc++.so.6
# ещё нет: слинкованное не запускается вовсе.
#
# Лечится это RPATH на каталог libstdc++ САМОГО компилятора, а не статической
# линковкой. -static-libstdc++ здесь не годится: Qt6 собран системным
# компилятором и свою libstdc++.so.6 тянет всё равно, так что в процессе
# оказываются ДВЕ копии рантайма. Они перебивают символы друг друга, и первым
# же падает инициализатор потоков ввода-вывода -- ios_base_library_init()
# уходит по нулевому адресу ещё до main().
#
# RPATH оставляет в процессе одну копию, и притом новую: она содержит и все
# старые версии символов, которые нужны Qt.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.so
    OUTPUT_VARIABLE CXX_RUNTIME_LIB
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
  if(CXX_RUNTIME_LIB AND IS_ABSOLUTE "${CXX_RUNTIME_LIB}")
    cmake_path(SET CXX_RUNTIME_LIB NORMALIZE "${CXX_RUNTIME_LIB}")
    cmake_path(GET CXX_RUNTIME_LIB PARENT_PATH CXX_RUNTIME_DIR)
    # Системный каталог прописывать незачем -- он и так в путях загрузчика.
    if(NOT CXX_RUNTIME_DIR MATCHES "^/(usr/)?lib(64)?$")
      message(STATUS "libstdc++ компилятора не системная, RPATH: ${CXX_RUNTIME_DIR}")
      list(APPEND CMAKE_BUILD_RPATH "${CXX_RUNTIME_DIR}")
      list(APPEND CMAKE_INSTALL_RPATH "${CXX_RUNTIME_DIR}")
    endif()
  endif()
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# ##############################################################################
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)

set(CLIPPER2_UTILS OFF)
set(CLIPPER2_EXAMPLES OFF)
set(CLIPPER2_TESTS OFF)
set(CLIPPER2_USINGZ "ONLY")

fetchcontent_declare(
  ctre
  GIT_REPOSITORY
    "https://github.com/hanickadot/compile-time-regular-expressions.git"
  GIT_TAG main
  GIT_SHALLOW ON)
fetchcontent_declare(
  clipper2
  GIT_REPOSITORY "https://github.com/XRay3D/Clipper2.git"
  GIT_TAG main
  GIT_SHALLOW ON
  SOURCE_SUBDIR CPP)
fetchcontent_makeavailable(ctre clipper2)

# ##############################################################################

option(GERBER_NATIVE_MARCH
       "Optimize for host CPU architecture (disables portability)" ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_compile_definitions(DEBUG=1)
endif()

# SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
# SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

add_compile_definitions(
  QT_MESSAGELOGCONTEXT
  QT_NO_CAST_TO_ASCII
  QT_NO_CAST_FROM_ASCII
  QT_NO_URL_CAST_FROM_STRING
  QT_NO_CAST_FROM_BYTEARRAY
  QT_USE_QSTRINGBUILDER
  QT_USE_FAST_OPERATOR_PLUS
  QT_DISABLE_DEPRECATED_BEFORE=0x070000
  saturate_cast=saturating_cast
  USINGZ=1)

# Compile options for our own code only — заданы после FetchContent, чтобы
# сторонние цели (Clipper2, ctre) их не наследовали. Clipper2 собирается со
# своим -Werror и ломался на диагностике _FORTIFY_SOURCE, протекавшей отсюда.
set(COMMON_COMPILE_OPTIONS
    -fPIC
    -Wall
    -Wextra
    -Wimplicit-fallthrough
    -Wpedantic
    # -Og instead of the default -O0 in Debug: _FORTIFY_SOURCE (pulled in by
    # -fhardened) requires optimizations, otherwise GCC emits -Whardened.
    $<$<CONFIG:Debug>:-Og>)

if(GERBER_NATIVE_MARCH)
  list(APPEND COMMON_COMPILE_OPTIONS -march=native)
endif()

# -fhardened, -fstack-clash-protection и RELRO/BIND_NOW завязаны на специфику
# glibc/ELF и не поддерживаются mingw-w64 GCC (Windows) и Homebrew GCC (macOS),
# поэтому включаем их только под Linux.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(
    ${COMMON_COMPILE_OPTIONS}
    # -fcf-protection=full -fvtable-verify=preinit
    -freflection -fstack-protector-strong)
  if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    add_compile_options(-fhardened -fstack-clash-protection)
    add_link_options(-Wl,-z,relro -Wl,-z,now)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(
    ${COMMON_COMPILE_OPTIONS}
    # -fcf-protection=full -fvtable-verify=preinit
    -freflection -fstack-protector-strong)
  if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    add_compile_options(-fhardened -fstack-clash-protection)
    add_link_options(-Wl,-z,relro -Wl,-z,now)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # Classic icc/icpc: no C++26 reflection support, but the driver is
  # GCC-compatible enough to accept the same hardening flags.
  add_compile_options(${COMMON_COMPILE_OPTIONS} -fstack-protector-strong)
  if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    add_link_options(-Wl,-z,relro -Wl,-z,now)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # No C++26 reflection support yet. /GS (stack protector) is already on by
  # default; approximate the rest of -fhardened with CFG + SDL checks.
  add_compile_options(/sdl /guard:cf)
  add_link_options(/DYNAMICBASE /NXCOMPAT /GUARD:CF)
endif()

# FIND BOOST — version check is handled by find_package directly
find_package(Boost 1.85 REQUIRED)
message(STATUS "==> Boost ${Boost_VERSION} found")

find_package(
  Qt6
  COMPONENTS Widgets LinguistTools Qml
  REQUIRED)

include(${CMAKE_SOURCE_DIR}/PATHS.cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(ggeasy)

# STATIC LIBS
add_subdirectory(static_libs)

# PLUGINS
add_subdirectory(plugins)

# MAIN APP
add_subdirectory(ggeasy)

# add_subdirectory(ToolTest)
