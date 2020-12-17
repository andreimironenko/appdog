#include <stdexcept>

#include <syslog.h>
#include <time.h>

#include "timer_.h"

void timer::timer_::signal_handler(int sig, siginfo_t *si, void *uc)
{
  auto tm = static_cast<timer_*>(si->si_value.sival_ptr);
  if (tm && sig == tm->_signal)
  {
    tm->_callback(tm->_data);
  }
}

timer::timer_::timer_(std::chrono::duration<long> period_sec,
    callback_t callback, void* data,
    bool is_single_shot, int sig
    ):
  _period(period_sec),
  _callback(callback),
  _data(data),
  _is_single_shot(is_single_shot),
  _signal(sig)
  {
  _sa.sa_flags = SA_SIGINFO;
  _sa.sa_sigaction = signal_handler;
  sigemptyset(&_sa.sa_mask);

  if (sigaction(SIGRTMAX, &_sa, NULL) == -1)
  {
    syslog(LOG_INFO, "call of sigaction(SIGRTMAX) is failed");
    return;
  }

  /* Create and start one timer for each command-line argument */
  _sev.sigev_notify = SIGEV_SIGNAL;    /* Notify via signal */
  _sev.sigev_signo = sig;         /* Notify using this signal*/
  _sev.sigev_value.sival_ptr = this;

  _ts.it_value.tv_sec = _period.count();
  _ts.it_value.tv_nsec = 0;

  if (!_is_single_shot) {
    _ts.it_interval.tv_sec = _period.count();
    _ts.it_interval.tv_nsec = 0;
  }

  /* Allows handler to get ID of this timer */
  if (timer_create(CLOCK_REALTIME, &_sev, &_timer) == -1)
  {
    throw std::runtime_error("Failed to create a timer");
  }

  if (timer_settime(_timer, 0, &_ts, NULL) == -1)
  {
    throw std::runtime_error("Failed to start timer");
  }
}

timer::timer_::~timer_()
{
  suspend();
  timer_delete(_timer);
}

void timer::timer_::suspend()
{
  _ts.it_value.tv_sec =  0;
  _ts.it_value.tv_nsec = 0;
  _ts.it_interval.tv_sec = 0;
  _ts.it_interval.tv_nsec = 0;

  if (timer_settime(_timer, 0, &_ts, NULL) == -1)
  {
    throw std::runtime_error("Failed to start timer");
  }
}

void timer::timer_::resume()
{
  _ts.it_value.tv_sec = _period.count();
  _ts.it_value.tv_nsec = 0;

  if (!_is_single_shot) {
    _ts.it_interval.tv_sec = _period.count();
    _ts.it_interval.tv_nsec = 0;
  }

  if (timer_settime(_timer, 0, &_ts, NULL) == -1)
  {
    throw std::runtime_error("Failed to start timer");
  }
}
