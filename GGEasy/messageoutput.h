#pragma once
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/

#include <QMessageLogContext>

#define ATTRIBUTES_OFF() "\033[m"

#define SET_FOREGROUND_COLOR(R, G, B) "\033[38;2" \
                                      ";" #R      \
                                      ";" #G      \
                                      ";" #B "m"

#define SET_BACKGROUND_COLOR(R, G, B) "\033[48;2" \
                                      ";" #R      \
                                      ";" #G      \
                                      ";" #B "m"

//    ANSI escape color codes :
#define BG_BLACK() "\033[40m"
#define BG_BLUE() "\033[44m"
#define BG_CYAN() "\033[46m"
#define BG_GREEN() "\033[42m"
#define BG_MAGENTA() "\033[45m"
#define BG_RED() "\033[41m"
#define BG_WHITE() "\033[47m"
#define BG_YELLOW() "\033[43m"
#define FG_BLACK() "\033[30m"
#define FG_BLUE() "\033[34m"
#define FG_CYAN() "\033[36m"
#define FG_GREEN() "\033[32m"
#define FG_MAGENTA() "\033[35m"
#define FG_RED() "\033[31m"
#define FG_WHITE() "\033[37m"
#define FG_YELLOW() "\033[33m"

#define BG_BRIGHT_BLACK() "\033[100m"
#define BG_BRIGHT_BLUE() "\033[104m"
#define BG_BRIGHT_CYAN() "\033[106m"
#define BG_BRIGHT_GREEN() "\033[102m"
#define BG_BRIGHT_MAGENTA() "\033[105m"
#define BG_BRIGHT_RED() "\033[101m"
#define BG_BRIGHT_WHITE() "\033[107m"
#define BG_BRIGHT_YELLOW() "\033[103m"
#define FG_BRIGHT_BLACK() "\033[90m"
#define FG_BRIGHT_BLUE() "\033[94m"
#define FG_BRIGHT_CYAN() "\033[96m"
#define FG_BRIGHT_GREEN() "\033[92m"
#define FG_BRIGHT_MAGENTA() "\033[95m"
#define FG_BRIGHT_RED() "\033[91m"
#define FG_BRIGHT_WHITE() "\033[97m"
#define FG_BRIGHT_YELLOW() "\033[93m"

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
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

    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(127, 255, 255) FG_BLACK() "Debug" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 255, 0) FG_BLACK() "Info" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 0, 255) FG_BLACK() "Warning" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 0, 0) FG_BLACK() "Critical" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 0, 0) FG_BLACK() "Fatal" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    }
}
