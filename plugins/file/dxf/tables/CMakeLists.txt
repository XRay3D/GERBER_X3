#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  01 February 2020                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2022                                          *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*******************************************************************************/
set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_abstracttable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_layer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_layermodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_style.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_appid.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_block_record.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_dimstyle.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ltype.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ucs.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_view.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_vport.cpp
    PARENT_SCOPE
)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_abstracttable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_layer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_layermodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_style.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_appid.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_block_record.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_dimstyle.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ltype.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ucs.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_view.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_vport.h
    PARENT_SCOPE
)
