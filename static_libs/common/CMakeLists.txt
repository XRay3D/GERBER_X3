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

project(common LANGUAGES CXX)

find_package(Qt6 COMPONENTS Qml REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n)

file(GLOB HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC ICU::i18n
           ICU::uc
           filetree
           gi
           shape
           $<$<PLATFORM_ID:Linux>:tbb>
           tooldatabase
           Qt6::Qml
           Qt6::Widgets)

target_setup_translations(${PROJECT_NAME} ${HEADERS} ${SOURCES})

# add_subdirectory(doublespinbox)
