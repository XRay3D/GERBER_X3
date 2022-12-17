#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  03 October 2022                                                 *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2022                                          *
#* License   :                                                                  *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#********************************************************************************/
cmake_minimum_required(VERSION 3.20)


project(filetree LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

add_library(filetree STATIC
    ft_foldernode.cpp
    ft_foldernode.h
    ft_model.cpp
    ft_model.h
    ft_node.cpp
    ft_node.h
    ft_radiodelegate.cpp
    ft_radiodelegate.h
    ft_sidedelegate.cpp
    ft_sidedelegate.h
    ft_textdelegate.cpp
    ft_textdelegate.h
    ft_typedelegate.cpp
    ft_typedelegate.h
    ft_view.cpp
    ft_view.h
)

target_link_libraries(filetree PRIVATE
    gi
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

#target_compile_definitions(filetree PRIVATE FILETREE_LIBRARY)
