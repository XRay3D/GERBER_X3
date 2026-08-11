# Geo -- геометрия и вычисления, отдельно от разбора DXF и от GUI.
#
# Наружу торчат только заголовки из include/Geo: плоский bulge-вид
# (Polyline), обёртки над точным регионом (Polygon/Polygons), раздувание с
# булевыми операциями (Boolean) и восстановление дуг (ArcRecovery). CGAL
# подключается ЗДЕСЬ и целиком остаётся внутри -- в публичных заголовках
# его нет вовсе, точное состояние спрятано за PIMPL (std::indirect).

# Геометрия -- точная, на CGAL (Boolean set operations над дуговыми
# полигонами). Плавающих движков на этой ветке нет вовсе.
find_package(CGAL QUIET)
find_package(Threads REQUIRED)

add_library(
  Geo STATIC
  include/geo/arcrecovery.h
  include/geo/boolean.h
  include/geo/cancel.h
  include/geo/polygon.h
  include/geo/polyline.h
  include/geo/util.h
  src/arcrecovery.cpp
  src/boolean.cpp
  src/cancel.cpp
  src/cgal.cpp
  src/cgal.h
  src/clipopen.cpp
  src/offsetcapsules.h
  src/polygon.cpp
  src/polyline.cpp
  src/util.cpp)
add_library(Geo::Geo ALIAS Geo)

# include/ -- наружу, src/ -- только своим: cgal.h и offsetcapsules.h
# тянут за собой EPECK, и вызывающему они не нужны.
target_include_directories(
  Geo
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Qt6::Gui, а не Widgets: из Qt здесь только QPointF/QRectF и QPainterPath.
target_link_libraries(Geo PUBLIC Qt6::Gui Threads::Threads simdjson)

# Обработчик нарушений контрактов (pre()/post()) живёт в libstdc++exp, и без
# него не слинкуется НИЧТО, что тянет заголовки Geo: контракты инстанцируются
# у вызывающего. Потому PUBLIC, а не PRIVATE.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(Geo PUBLIC stdc++exp)
endif()

if(CGAL_FOUND)
  target_link_libraries(Geo PRIVATE CGAL::CGAL)
else()
  # CGAL header-only: достаточно самих gmp/mpfr -- но только если заголовки
  # реально лежат в путях компилятора по умолчанию. find_package зовём QUIET
  # (config-файла может не быть вовсе), и без этой проверки отсутствие CGAL
  # ничем не проявляется до самой сборки: конфигурация проходит, а потом
  # каждый .cpp падает на <CGAL/...> -- ровно так и сломался Windows-раннер.
  #
  # Проба ТОЛЬКО компилирует (STATIC_LIBRARY): линковать её нечем и незачем.
  # Заголовок ядра инстанцирует обёртки над gmp, так что пробе понадобились бы
  # -lgmp -lmpfr, а с mold она вдобавок утыкается в .debug_gdb_scripts (см.
  # ниже) -- и «нет CGAL» получалось бы там, где он есть. Спрашиваем ровно то,
  # что ломается: видны ли заголовки.
  block()
    include(CheckIncludeFileCXX)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
    check_include_file_cxx("CGAL/Exact_predicates_exact_constructions_kernel.h"
                           HAVE_CGAL_HEADERS)
    set(HAVE_CGAL_HEADERS "${HAVE_CGAL_HEADERS}" PARENT_SCOPE)
  endblock()
  if(NOT HAVE_CGAL_HEADERS)
    message(
      FATAL_ERROR
        "Не найден CGAL -- без него static_libs/geo не собрать.\n"
        "  MSYS2/UCRT64: pacman -S mingw-w64-ucrt-x86_64-cgal\n"
        "  Debian/Ubuntu: apt install libcgal-dev\n"
        "  ALT/Fedora: dnf install CGAL-devel\n"
        "  macOS: brew install cgal")
  endif()
  target_link_libraries(Geo PRIVATE gmp mpfr)
endif()

# CGAL 6.2 вставляет в объектный файл inline-скрипт автозагрузки своих
# печатальщиков для gdb (include/CGAL/gdb_autoload.h) -- секцией
# .debug_gdb_scripts с флагами MERGE|STRINGS, но БЕЗ завершающего нуля.
# GNU ld и lld это терпят, а mold отказывается линковать вовсе:
# "string is not null terminated". Поскольку CGAL компилируется только здесь,
# хватает выключателя на этой цели -- и только для mold, чтобы на прочих
# линкерах печатальщики остались на месте. Убрать, когда CGAL починит секцию.
if(CMAKE_LINKER_TYPE STREQUAL "MOLD")
  target_compile_definitions(Geo PRIVATE CGAL_NO_GDB_AUTOLOAD)
endif()

# Объединение регионов идёт в несколько потоков (joinAll в src/cgal.cpp).
# Без CGAL_HAS_THREADS CGAL считает себя однопоточным и правит счётчики
# ссылок точных чисел без атомарности -- гонка на ровном месте. Boost обычно
# определяет его сам, но полагаться на это здесь нельзя.
target_compile_definitions(Geo PRIVATE CGAL_HAS_THREADS)

# У CGAL СВОЙ переключатель assert/precondition -- CGAL_NDEBUG, независимый
# от общего NDEBUG (и потому не гасится обычным Release). Без него
# Gps_on_surface_base_2::_join() после КАЖДОГО join/difference прогоняет
# CGAL_assertion(is_valid()) -- полную проверку всего разбиения плоскости
# точечной локализацией (вертикальный луч на каждую грань), а не только
# входных контуров. На дереве слияний (joinAll) это множится на число узлов:
# файл с полутысячей дуговых фигур от этого превращается из «доли секунды»
# в «десятки секунд на КАЖДОЕ join» -- в бэктрейсе рабочий поток стоит
# именно в CGAL::is_valid, а не в самом объединении.
#
# Входные контуры уже провалидированы своими средствами -- toGPoly() зовёт
# is_valid_unknown_polygon() сам и полагается на сверку площади с bulge-видом
# (см. cgal.cpp). Эта проверка CGAL -- избыточный дубль их же алгоритма
# поверх готового результата, а не отдельная гарантия.
target_compile_definitions(Geo PRIVATE CGAL_NDEBUG)

# Интервальная арифметика CGAL требует честного режима округления. Только
# здесь: точные вычисления целиком внутри этой библиотеки и остались.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(Geo PRIVATE -frounding-math)
endif()

# Свои тесты -- рядом с самой библиотекой: проверяют её же публичный API,
# а не приложение поверх него.
option(GEO_BUILD_TESTS "Собирать тесты библиотеки Geo" OFF #[[ON]])
if(GEO_BUILD_TESTS)
  add_subdirectory(tests)
endif()
