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

# Классы ядра с сигналами помечены GGCORE_EXPORT (ggcore_export.h): для
# потребителей на Windows это dllimport — иначе PMF-указатель на сигнал,
# взятый в exe/плагине, указывает на локальный thunk импортной библиотеки,
# и connect() через границу DLL не находит сигнал. Внутри самого ядра макрос
# обязан быть пуст (dllexport выключил бы auto-export mingw-ld), поэтому
# GGCORE_BUILD ставится всем статлибам, попадающим в ggcore. Транзитивная
# geometry -- алиас Geo (static_libs/CMakeLists.txt), она уже в списке;
# на алиас target_compile_definitions звать нельзя.
if(WIN32)
  foreach(lib IN LISTS GGCORE_LIBS)
    target_compile_definitions(${lib} PRIVATE GGCORE_BUILD)
  endforeach()
endif()

# На MinGW экспортная таблица DLL строится авто-экспортом mingw-ld; он
# совместим с whole-archive только без LTO-объектов в архивах (иначе
# "cannot export ...: symbol wrong type (4 vs 3)"), поэтому LTO под MinGW
# выключен глобально -- см. корневой CMakeLists у fetchcontent_makeavailable.
# WINDOWS_EXPORT_ALL_SYMBOLS здесь не работает: nm гоняется только по
# собственным объектникам цели (у ggcore -- одна пустышка), символы из
# линкуемых архивов в .def не попадают.

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
