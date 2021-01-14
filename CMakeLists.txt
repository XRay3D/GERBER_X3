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
cmake_minimum_required(VERSION 3.14)

project(Gerber_X3)

# static lib
add_subdirectory(static_libs)

# main app
add_subdirectory(GGEasy)

# plugins file
#add_subdirectory(file_plugins)

# plugins shape
add_subdirectory(shape_plugins)

## QT4_WRAP_TS(lupdate_outputs lrelease_outputs prefix lang1 lang2 ... )
#MACRO (QT4_WRAP_TS lupdate_outputs lrelease_outputs prefix)
#    FOREACH (it ${ARGN})
#      SET(tsfile ${CMAKE_CURRENT_SOURCE_DIR}/translations/${prefix}_${it}.ts)
##      SET(tsfile ${CMAKE_CURRENT_BINARY_DIR}/translations/${prefix}_${it}.ts)
#      ADD_CUSTOM_COMMAND(OUTPUT ${tsfile}
#        COMMAND ${QT_LUPDATE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} -ts ${tsfile}
#      )

#      SET(qmfile ${CMAKE_CURRENT_BINARY_DIR}/${prefix}_${it}.qm)
#      ADD_CUSTOM_COMMAND(OUTPUT ${qmfile}
#        COMMAND ${QT_LRELEASE_EXECUTABLE} ${tsfile} -qm ${qmfile}
#      )

#      SET(${lupdate_outputs} ${${lupdate_outputs}} ${tsfile})
#      SET(${lrelease_outputs} ${${lrelease_outputs}} ${qmfile})
#    ENDFOREACH(it)
#ENDMACRO (QT4_WRAP_TS)


## Create our translation files.
#QT4_WRAP_TS(lupdate_outputs lrelease_outputs
#  foo
#  en_AU en_US en_GB fr_FR de_DE ja_JM zh_CN
#)

## We only want to run lupdate manually (need to check out all the .ts files),
## and this is typically only done when the UI text has changed.  So we don't
## include lupdate as part of the default build target.
## But we want to run lrelease every time, so the lrelease outputs will become
## part of the windows_installer target (which will in turn be part of the
## default build target).
#ADD_CUSTOM_TARGET(lupdate
#  DEPENDS ${lupdate_outputs}
#)
