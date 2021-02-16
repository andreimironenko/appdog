#include <sys/types.h>

#include <ratio>
#include <chrono>

#include "messages/message.h"

#pragma once

using std::chrono::duration;
using namespace std::chrono_literals;


namespace appdog::messages {

  struct activate : public message
  {
    friend void to_json(json& j, const activate& msg);
    friend void from_json(const json& j, activate& msg);

    long _reset_time;
    int _signal;
    long _delay_after_sigterm;

    activate(pid_t pid=0, long tid = 0, duration<long, std::nano> reset_time=60s,
        int sig=SIGTERM, duration<long, std::nano> delay_after_sigterm=0s)
      :
      message(msg_id::ACTIVATE, pid, tid),
      _reset_time(reset_time.count()),
      _signal(sig),
      _delay_after_sigterm(delay_after_sigterm.count())
    {}


    friend bool operator==(const activate& lhs, const activate& rhs)
    {
      if (!(dynamic_cast<const message&>(lhs) == dynamic_cast<const message&>(rhs)))
      {
        return false;
      }

      if (lhs._reset_time != rhs._reset_time || lhs._signal != rhs._signal ||
          lhs._delay_after_sigterm != rhs._delay_after_sigterm)
      {
        return false;
      }
      return true;
    }
  };

  void to_json(json& j, const activate& msg)
  {
    j = json{{"id", msg._id}, {"pid", msg._pid}, {"tid", msg._tid},
      {"reset_time", msg._reset_time}, {"signal", msg._signal},
      {"delay_after_sigterm", msg._delay_after_sigterm}
    };
  }

  void from_json(const json& j, activate& msg)
  {
    from_json(j, dynamic_cast<message&>(msg));
    j.at("reset_time").get_to(msg._reset_time);
    j.at("signal").get_to(msg._signal);
    j.at("delay_after_sigterm").get_to(msg._delay_after_sigterm);
  }
} // namespace appdog::messages
