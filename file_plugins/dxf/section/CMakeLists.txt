#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  01 February 2020                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2021                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_blocks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_entities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_headerparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_sectionparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_tables.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_classes.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_objects.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_thumbnailimage.cpp
    PARENT_SCOPE
)
set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_blocks.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_entities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_headerparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_sectionparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_tables.h
    ${HEADERS}
#    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_classes.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_objects.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_thumbnailimage.h
    PARENT_SCOPE
)


