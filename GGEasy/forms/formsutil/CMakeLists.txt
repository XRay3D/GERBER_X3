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
