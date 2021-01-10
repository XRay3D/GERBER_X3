cmake_minimum_required(VERSION 3.14)

set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/drillform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drillmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drillform.ui
    PARENT_SCOPE
    )

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/drillform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drillmodel.h
    PARENT_SCOPE
    )

set(UI
    ${UI}
    ${CMAKE_CURRENT_SOURCE_DIR}/drillform.ui
    PARENT_SCOPE
    )

