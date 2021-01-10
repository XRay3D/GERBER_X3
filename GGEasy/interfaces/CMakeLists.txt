cmake_minimum_required(VERSION 3.14)

set(SOURCE
    ${SOURCE}
    PARENT_SCOPE
    )

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/file.h
    ${CMAKE_CURRENT_SOURCE_DIR}/node.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pluginfile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugintypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shapepluginin.h
    PARENT_SCOPE
    )

set(UI
    ${UI}
    PARENT_SCOPE
    )
