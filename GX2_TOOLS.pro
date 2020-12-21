TEMPLATE = subdirs

SUBDIRS += GGEasy
win32-msvc*{
#    SUBDIRS += GTE_Win
}

DISTFILES += \
    gerber_x2/GX2.xps \
    gerber_x2/the_gerber_file_format_specification.pdf \
    gerber_x2/the_gerber_file_format_version_2_faq.pdf \
    gerber_x2/Window.PNG \
    gerber_x2/clipper_ver6.4.2.zip

