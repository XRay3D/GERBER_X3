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
target_link_libraries(Geo PUBLIC Qt6::Gui Threads::Threads)

# Обработчик нарушений контрактов (pre()/post()) живёт в libstdc++exp, и без
# него не слинкуется НИЧТО, что тянет заголовки Geo: контракты инстанцируются
# у вызывающего. Потому PUBLIC, а не PRIVATE.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(Geo PUBLIC stdc++exp)
endif()

if(CGAL_FOUND)
  target_link_libraries(Geo PRIVATE CGAL::CGAL)
else()
  # CGAL header-only: достаточно самих gmp/mpfr.
  target_link_libraries(Geo PRIVATE gmp mpfr)
endif()

# Объединение регионов идёт в несколько потоков (joinAll в src/cgal.cpp).
# Без CGAL_HAS_THREADS CGAL считает себя однопоточным и правит счётчики
# ссылок точных чисел без атомарности -- гонка на ровном месте. Boost обычно
# определяет его сам, но полагаться на это здесь нельзя.
target_compile_definitions(Geo PRIVATE CGAL_HAS_THREADS)

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
