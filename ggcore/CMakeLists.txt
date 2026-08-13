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
cmake_minimum_required(VERSION 4.0)

project(ggcore LANGUAGES CXX)

find_package(
    Qt6
    COMPONENTS Widgets Qml OpenGL OpenGLWidgets
    REQUIRED)

# Общее ядро приложения одной shared-библиотекой: раньше каждый из 14
# плагинов статически линковал весь этот набор (вместе с CGAL/simdjson это
# ~5.5 МБ на плагин, ~70 МБ дублей на комплект). Теперь ядро существует в
# процессе одной копией, а плагины и GGEasy линкуют его динамически.
set(GGCORE_LIBS
    Geo
    common
    gi
    graphicsview
    tooldatabase
    filetree
    file_plugins
    gcode
    shape)

add_library(${PROJECT_NAME} SHARED ggcore_dummy.cpp)

if(MINGW)
  # Тот же класс проблем, что и у сторонних целей под LTO на MinGW (см.
  # комментарий в корневом CMakeLists про FetchContent и "COFF object format
  # mismatch"): здесь mingw-ld на этапе auto-export падает на десятках
  # шаблонных weak-символов (std::ranges::zip_view, std::variant,
  # Geo::Polyline...) с "cannot export ...: symbol wrong type (4 vs 3)".
  # LTO мержит девять WHOLE_ARCHIVE-архивов через параллельные LTRANS-
  # партиции, и один и тот же weak-символ получает в разных партициях
  # разный тип (функция/данные) -- auto-export эту рассинхронизацию не
  # переживает. Отключаем LTO только для ggcore: остальной Release (в т.ч.
  # Linux) LTO не теряет.
  set_target_properties(${PROJECT_NAME} PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

# WHOLE_ARCHIVE: в .so нужны ВСЕ объекты статлиб (включая moc- и rcc-объекты
# с Q_CONSTRUCTOR_FUNCTION, например gc_scripts.qrc), а не только те, на
# которые ссылается сам ggcore — на остальные ссылаются плагины, и обычная
# линковка отбросила бы их до dlopen. simdjson здесь по той же причине.
list(JOIN GGCORE_LIBS "," GGCORE_LIBS_COMMA)
target_link_libraries(${PROJECT_NAME}
    PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,${GGCORE_LIBS_COMMA},simdjson>")

# Потребителям (GGEasy, плагины) — только compile-требования статлиб
# (заголовки, дефайны, транзитивные Qt/ICU): линковать их архивы нельзя,
# иначе всё дублирование вернётся обратно.
foreach(lib IN LISTS GGCORE_LIBS)
    target_link_libraries(${PROJECT_NAME} INTERFACE $<COMPILE_ONLY:${lib}>)
endforeach()

# Header-only ctre и stdc++exp инстанцируются на стороне вызывающего.
# Qt-модули — INTERFACE: $<COMPILE_ONLY:...> выше отдаёт только заголовки,
# а плагины собираются с -z,defs и обязаны долинковать Qt-DSO сами.
target_link_libraries(${PROJECT_NAME} INTERFACE ctre
    $<$<CXX_COMPILER_ID:GNU>:stdc++exp>
    Qt6::Widgets Qt6::Qml Qt6::OpenGL Qt6::OpenGLWidgets)

# Строго рядом с GGEasy, НЕ в PLUGINS_DIR: main.cpp грузит QPluginLoader'ом
# все plugins/*.so подряд и попытался бы загрузить ядро как плагин.
# RUNTIME_OUTPUT_DIRECTORY — для DLL на MinGW.
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
