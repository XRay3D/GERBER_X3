cmake_minimum_required(VERSION 4.00)

project(doublespinbox VERSION 0.1 LANGUAGES CXX)





#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 #[[Qt5]] REQUIRED COMPONENTS Widgets)

find_package(Qt6 REQUIRED COMPONENTS Widgets Qml)

set(PROJECT_SOURCES
        main.cpp
        ../doublespinbox.cpp
        ../doublespinbox.h
        # ../mathparser.cpp
        # ../mathparser.h
        # widget.cpp
        # widget.h
        # widget.ui
)

include_directories(../)

    qt_add_executable(doublespinbox
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET doublespinbox APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

target_link_libraries(doublespinbox PRIVATE Qt6::Widgets Qt6::Qml)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.doublespinbox)
endif()
set_target_properties(doublespinbox PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS doublespinbox
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

    qt_finalize_executable(doublespinbox)
