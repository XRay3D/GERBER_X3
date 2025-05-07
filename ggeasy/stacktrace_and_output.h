#pragma once

#include "mainwindow.h"

#include <QDebug>
#include <app.h>
#include <csignal>
#include <string>

#if __cpp_lib_stacktrace_
#include <QMessageLogContext>
#include <stacktrace>
#define STACKTRACE std::to_string(std::stacktrace::current())
#elif __has_include(<boost/stacktrace.hpp>)
#include <boost/stacktrace.hpp>
namespace bs = boost::stacktrace;
#define STACKTRACE bs::to_string(bs::stacktrace())
#else

#include <cxxabi.h>
#include <execinfo.h> // linux
#include <sys/wait.h> // linux

// std::string print_trace() {
//     char pid_buf[30];
//     sprintf(pid_buf, "%d", getpid());
//     char name_buf[512];
//     name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
//     int child_pid = fork();
//     if(!child_pid) {
//         dup2(2, 1); // redirect output to stderr
//         fprintf(stdout, "stack trace for %s pid=%s\n", name_buf, pid_buf);
//         execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, NULL);
//         abort(); /* If gdb failed to start */
//     } else {
//         waitpid(child_pid, NULL, 0);
//     }
//     return {};
// }

inline std::string print_trace1() {
    int nptrs;
    void* buffer[1000]{};

    nptrs = backtrace(buffer, std::size(buffer));
    qWarning("backtrace() returned %d addresses\n", nptrs);

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
              would produce similar output to the following: */

    char** strings = backtrace_symbols(buffer, nptrs);
    if(strings == nullptr) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    std::string str;
    for(size_t j = 0; j < nptrs; j++) {
        // qCritical("%s", strings[j]);
        QString string{strings[j]};

        std::string_view sv{strings[j]};
        auto b = sv.find_first_of('(') + 1;
        auto e = sv.find_first_of('+', b);
        sv = {sv.begin() + b, sv.begin() + e};
        std::string mangle{sv};

        if(mangle.size()) {
            int status{};
            char* realname = abi::__cxa_demangle(mangle.data(), NULL, NULL, &status);
            if(!status)
                string.replace(mangle.data(), realname);
            std::free(realname);
        }
        str += string.toStdString() + '\n';
    }

    free(strings);
    return str;
}

#define STACKTRACE print_trace1()
#endif

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

    std::string str{STACKTRACE};

    // str = std::regex_replace(str, std::regex(R"(C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.40.33807\\include\\)"), "Sys ==> ");
    // str = std::regex_replace(str, std::regex(R"(C:\\Users\\bakiev\\Nextcloud\\IKSU_3000_AUTO\\IKSU_3000_HANDLE\\)"), "XR ==> ");

    // MessageBoxA(NULL, str.str().c_str(), "Exception catched: SIGSEGV (segment violation)!", NULL);
    // QMessageBox::critical(nullptr, "Exception catched: SIGSEGV (segment violation)!", QString::fromStdString(str.str()));
    qCritical("%s\n%s", SIG(), str.c_str());

    exit(-signum);
    // signal(signum, SIG_DFL);
}

inline auto messageHandler = qInstallMessageHandler(nullptr);
inline void myMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    QMessageLogContext& context_ = const_cast<QMessageLogContext&>(context);

    if(context.file) {
        std::string_view file{context.file};
        static constexpr auto delim = R"(\/)"sv;
        if(auto last = file.find_last_of(delim); last != std::string_view::npos)
            if(last = file.find_last_of(delim, last - 1); last != std::string_view::npos)
                context_.file = file.data() + last;
    }

    if(App::mainWindowPtr())
        App::mainWindow().logMessage2(type, context, message);
    messageHandler(type, context, message);
}

inline void stacktraceAndOutput() {
    qInstallMessageHandler(myMessageHandler);
    qSetMessagePattern(QLatin1String(
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
        "\x1b[38;2;64;64;64m <- %{function} <- %{file} : %{line}\x1b[0m"));

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
