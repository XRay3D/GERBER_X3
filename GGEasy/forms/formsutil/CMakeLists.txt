#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  14 January 2021                                                 *
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
    ${CMAKE_CURRENT_SOURCE_DIR}/errordialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/formsutil.cpp
    PARENT_SCOPE
    )

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/errordialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/formsutil.h
    PARENT_SCOPE
    )

set(UI
    ${UI}
    PARENT_SCOPE
    )
