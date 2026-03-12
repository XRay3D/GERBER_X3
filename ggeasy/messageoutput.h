#pragma once
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
// #include "a_pch.h"
#include <QMessageLogContext>

#define ATTRIBUTES_OFF() u"\033[m"_s

#define SET_FOREGROUND_COLOR(R, G, B) u"\033[38;2"_s \
                                      u";"_s #R      \
                                      u";"_s #G      \
                                      u";"_s #B u"m"_s

#define SET_BACKGROUND_COLOR(R, G, B) u"\033[48;2"_s \
                                      u";"_s #R      \
                                      u";"_s #G      \
                                      u";"_s #B u"m"_s

//    ANSI escape color codes :
#define BG_BLACK()   u"\033[40m"_s
#define BG_BLUE()    u"\033[44m"_s
#define BG_CYAN()    u"\033[46m"_s
#define BG_GREEN()   u"\033[42m"_s
#define BG_MAGENTA() u"\033[45m"_s
#define BG_RED()     u"\033[41m"_s
#define BG_WHITE()   u"\033[47m"_s
#define BG_YELLOW()  u"\033[43m"_s
#define FG_BLACK()   u"\033[30m"_s
#define FG_BLUE()    u"\033[34m"_s
#define FG_CYAN()    u"\033[36m"_s
#define FG_GREEN()   u"\033[32m"_s
#define FG_MAGENTA() u"\033[35m"_s
#define FG_RED()     u"\033[31m"_s
#define FG_WHITE()   u"\033[37m"_s
#define FG_YELLOW()  u"\033[33m"_s

#define BG_BRIGHT_BLACK()   u"\033[100m"_s
#define BG_BRIGHT_BLUE()    u"\033[104m"_s
#define BG_BRIGHT_CYAN()    u"\033[106m"_s
#define BG_BRIGHT_GREEN()   u"\033[102m"_s
#define BG_BRIGHT_MAGENTA() u"\033[105m"_s
#define BG_BRIGHT_RED()     u"\033[101m"_s
#define BG_BRIGHT_WHITE()   u"\033[107m"_s
#define BG_BRIGHT_YELLOW()  u"\033[103m"_s
#define FG_BRIGHT_BLACK()   u"\033[90m"_s
#define FG_BRIGHT_BLUE()    u"\033[94m"_s
#define FG_BRIGHT_CYAN()    u"\033[96m"_s
#define FG_BRIGHT_GREEN()   u"\033[92m"_s
#define FG_BRIGHT_MAGENTA() u"\033[95m"_s
#define FG_BRIGHT_RED()     u"\033[91m"_s
#define FG_BRIGHT_WHITE()   u"\033[97m"_s
#define FG_BRIGHT_YELLOW()  u"\033[93m"_s

inline void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    //    ANSI escape color codes :
    //    Name            FG  BG
    //    Black           30  40
    //    Red             31  41
    //    Green           32  42
    //    Yellow          33  43
    //    Blue            34  44
    //    Magenta         35  45
    //    Cyan            36  46
    //    White           37  47
    //    Bright Black    90  100
    //    Bright Red      91  101
    //    Bright Green    92  102
    //    Bright Yellow   93  103
    //    Bright Blue     94  104
    //    Bright Magenta  95  105
    //    Bright Cyan     96  106
    //    Bright White    97  107

    QByteArray localMsg = msg.toUtf8();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    switch(type) {
    case QtDebugMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(127, 255, 255) FG_BLACK() u"Debug"_s ATTRIBUTES_OFF() u": %s"_s //
            SET_FOREGROUND_COLOR(127, 127, 127) u"\n\t(%s:%u, %s)\n"_s ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 255, 0) FG_BLACK() u"Info"_s ATTRIBUTES_OFF() u": %s"_s //
            SET_FOREGROUND_COLOR(127, 127, 127) u"\n\t(%s:%u, %s)\n"_s ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 0, 255) FG_BLACK() u"Warning"_s ATTRIBUTES_OFF() u": %s"_s //
            SET_FOREGROUND_COLOR(127, 127, 127) u"\n\t(%s:%u, %s)\n"_s ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 0, 0) FG_BLACK() u"Critical"_s ATTRIBUTES_OFF() u": %s"_s //
            SET_FOREGROUND_COLOR(127, 127, 127) u"\n\t(%s:%u, %s)\n"_s ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 0, 0) FG_BLACK() u"Fatal"_s ATTRIBUTES_OFF() u": %s"_s //
            SET_FOREGROUND_COLOR(127, 127, 127) u"\n\t(%s:%u, %s)\n"_s ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    }
}
