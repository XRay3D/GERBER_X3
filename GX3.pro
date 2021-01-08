#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  01 February 2020                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2020                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
TEMPLATE = subdirs

# app
SUBDIRS += \
    GGEasy \#       main app
    clipper \#      static lib
    filetree \#     static lib
    gi \#           static lib
    graphicsview \# static lib
    project \#      static lib
    settings \#     static lib
    thermal \#      static lib
    tooldatabase \# static lib

# plugins
SUBDIRS += \
#    dxf \
#    excellon \
    gerber \

SUBDIRS += \
#    GTE_Win \


win32-msvc*{
#    SUBDIRS += GTE_Win
}

DISTFILES += \
    gerber_x2/GX2.xps \
    gerber_x2/the_gerber_file_format_specification.pdf \
    gerber_x2/the_gerber_file_format_version_2_faq.pdf \
    gerber_x2/Window.PNG \
    gerber_x2/clipper_ver6.4.2.zip

