#include "timer.h"
#include "timer_.h"

timer::timer(std::chrono::duration<long> period_sec,
     callback_t callback, void* data, bool is_single_shot, int sig) :
  _timer(new timer_(period_sec, callback, data, is_single_shot, sig))
{
}

timer::~timer()
{
}

void timer::start()
{
  _timer->resume();
}

void timer::suspend()
{
  _timer->suspend();
}

void timer::resume()
{
  _timer->resume();
}

void timer::stop()
{
  _timer->suspend();
}
