#pragma once

#include <exception>
#include <string>

class Cancel : public std::exception {
    std::string m_descr;

public:
    Cancel(const char* description)
        : m_descr{description} {
    }
    ~Cancel() noexcept override = default;
    const char* what() const noexcept override { return m_descr.c_str(); }
};

class ProgressCancel {
    /*thread_local*/ static inline int max_;
    /*thread_local*/ static inline int current_;
    /*thread_local*/ static inline bool cancel_;

public:
    static void reset() {
        current_ = 0;
        max_ = 0;
        cancel_ = 0;
    }

    static int max() { return max_; }
    static void setMax(int max) { max_ = max; }
    static int current() { return current_; }
    static void setCurrent(int current = 0) { current_ = current; }
    static void incCurrent() { ++current_; }
    static bool isCancel() { return cancel_; }
    static void ifCancelThenThrow() {
        ++current_;
        if(cancel_) [[unlikely]]
            throw Cancel{__FUNCTION__};
    }
    static void setCancel(bool cancel) { cancel_ = cancel; }
    static void cancel() { cancel_ = true; }
};

inline void ifCancelThenThrow() { ProgressCancel::ifCancelThenThrow(); }
