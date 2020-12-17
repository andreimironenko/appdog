#include <map>
#include <memory>
#include <ratio>
#include <chrono>
#include <ctime>

#include "timer.h"


#pragma once


  class timer::timer_
  {
    std::chrono::duration<long> _period;
    callback_t _callback;
    void* _data;
    bool _is_single_shot;
    int _signal;

    struct itimerspec _ts;
    struct sigaction _sa;
    struct sigevent _sev;

    timer_t _timer;

    //int signal() {return _signal;}
    //int timer() {return _timer;}

    public:
    static void signal_handler(int sig, siginfo_t *si, void *uc = nullptr);

    explicit timer_(std::chrono::duration<long> period_sec, callback_t callback, void* data,
        bool is_single_short, int sig);

    ~timer_();

    timer_(const timer_&) = delete;
    timer_(timer_&&) = delete;
    timer_& operator=(const timer_&) = delete;
    timer_& operator=(timer_&&) = delete;

    void suspend();
    void resume();
  };
