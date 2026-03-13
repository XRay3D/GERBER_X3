#pragma once

#include "mainwindow.h"

#include <QDebug>
#include <app.h>
#include <csignal>
#include <cxxabi.h>
#include <stacktrace>
#include <string>

#include <execinfo.h> // linux
#include <sys/wait.h> // linux

using namespace std::literals;

inline void death_signal(int signum) { // обработка Segfault
    auto SIG = [signum] {
        switch(signum) {
        case SIGABRT: return "SIGABRT";
        case SIGFPE: return "SIGFPE";
        case SIGILL: return "SIGILL";
        case SIGINT: return "SIGINT";
        case SIGSEGV: return "SIGSEGV";
        case SIGTERM: return "SIGTERM";
#ifdef _MSVC_LANG
        case SIGABRT_COMPAT: return "SIGABRT_COMPAT"; // MSVC ONLY?
        case SIGBREAK: return "SIGBREAK";             // MSVC ONLY?
#endif
        default: return "";
        }
    };

    std::string str{std::to_string(std::stacktrace::current())};

    // str = std::regex_replace(str, std::regex(uR"(C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.40.33807\\include\\)"), "Sys ==> ");
    // str = std::regex_replace(str, std::regex(uR"(C:\\Users\\bakiev\\Nextcloud\\IKSU_3000_AUTO\\IKSU_3000_HANDLE\\)"), "XR ==> ");

    // MessageBoxA(NULL, str.str().c_str(), "Exception catched: SIGSEGV (segment violation)!", NULL);
    // QMessageBox::critical(nullptr, "Exception catched: SIGSEGV (segment violation)!", QString::fromStdString(str.str()));
    qCritical("%s\n%s", SIG(), str.c_str());

    exit(-signum);
    // signal(signum, SIG_DFL);
}

inline const auto defaultMessageHandler = qInstallMessageHandler(nullptr);

inline void myMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    QMessageLogContext& context_ = const_cast<QMessageLogContext&>(context);

    if(message == u"Painter path exceeds +/-32767 pixels.") return;

    if(context.file) {
        std::string_view file{context.file};
        static constexpr auto delim = R"(\/)"sv;
        if(auto last = file.find_last_of(delim); last != std::string_view::npos)
            if(last = file.find_last_of(delim, last - 1); last != std::string_view::npos)
                context_.file = file.data() + last;
    }

    if(App::mainWindowPtr())
        App::mainWindow().logMessage2(type, context, message);
    defaultMessageHandler(type, context, message);
}

inline void stacktraceAndOutput() {
    qInstallMessageHandler(myMessageHandler);
    qSetMessagePattern(
        "%{if-critical}\x1b[38;2;255;0;0m"
        "C %{endif}"
        "%{if-debug}\x1b[38;2;196;196;196m"
        "D %{endif}"
        "%{if-fatal}\x1b[1;38;2;255;0;0m"
        "F %{endif}"
        "%{if-info}\x1b[38;2;128;255;255m"
        "I %{endif}"
        "%{if-warning}\x1b[38;2;255;128;0m"
        "W %{endif}"
        // "%{time HH:mm:ss.zzz} "
        // "%{appname} %{pid} %{threadid} "
        // "%{type} "
        // "%{file}:%{line} %{function} "
        "%{if-category}%{category}%{endif}%{message} "
        "\x1b[38;2;64;64;64m <- %{function} <- %{file} : %{line}\x1b[0m"_L1);

    // QApplication::setStyle(QStyleFactory::create("Windows 11"));
    signal(SIGABRT, death_signal);
    signal(SIGFPE, death_signal);
    signal(SIGILL, death_signal);
    signal(SIGINT, death_signal);
    signal(SIGSEGV, death_signal);
    signal(SIGTERM, death_signal);
#ifdef _MSVC_LANG
    signal(SIGABRT_COMPAT, death_signal); // MSVC ONLY?
    signal(SIGBREAK, death_signal);       // MSVC ONLY?
#endif
}
