#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  11 November 2021                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2021                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.14)

set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/gcodepropertiesform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hatchingform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketoffsetform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketrasterform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/profileform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/toolname.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/voronoiform.cpp
    PARENT_SCOPE
    )

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/gcodepropertiesform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hatchingform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketoffsetform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketrasterform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/profileform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/toolname.h
    ${CMAKE_CURRENT_SOURCE_DIR}/voronoiform.h
    PARENT_SCOPE
    )

set(UI
    ${UI}
    ${CMAKE_CURRENT_SOURCE_DIR}/gcodepropertiesform.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/hatchingform.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketoffsetform.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketrasterform.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/profileform.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/voronoiform.ui
    PARENT_SCOPE
    )


