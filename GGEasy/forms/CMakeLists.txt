cmake_minimum_required(VERSION 3.14)

set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/voronoiform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcodepropertiesform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketoffsetform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketrasterform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/profileform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/toolname.cpp
    PARENT_SCOPE
    )

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/gcodepropertiesform.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketoffsetform.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/pocketrasterform.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/profileform.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/voronoiform.ui
    PARENT_SCOPE
    )


