#include <sys/types.h>

#include "messages/message.h"

#pragma once

namespace appdog::messages {

  struct activate : public message
  {
    friend void to_json(json& j, const activate& msg);
    friend void from_json(const json& j, activate& msg);

    long _reset_time;
    int _signal;
    bool _enable_sigterm;
    long _delay_after_sigterm;

#if 0
    activate(pid_t pid, long tid = 0, long reset_time = 60, int signal = SIGKILL,
        bool enable_sigterm = false, long delay_after_sigterm = 0):
      message(msg_id::ACTIVATE, pid, tid),
      _reset_time(reset_time),
      _signal(signal),
      _enable_sigterm(enable_sigterm),
      _delay_after_sigterm(delay_after_sigterm)
    {}
#endif

    friend bool operator==(const activate& lhs, const activate& rhs)
    {
      if (!(dynamic_cast<const message&>(lhs) == dynamic_cast<const message&>(rhs)))
      {
        return false;
      }

      if (lhs._reset_time != rhs._reset_time || lhs._signal != rhs._signal ||
          lhs._enable_sigterm != rhs._enable_sigterm || lhs._delay_after_sigterm != rhs._delay_after_sigterm)
      {
        return false;
      }
      return true;
    }
  };

  void to_json(json& j, const activate& msg)
  {
    j = json{{"id", msg_id::ACTIVATE}, {"pid", msg._pid}, {"tid", msg._tid},
      {"reset_time", msg._reset_time}, {"signal", msg._signal}, {"enable_sigterm", msg._enable_sigterm},
      {"delay_after_sigterm", msg._delay_after_sigterm}
    };
  }

  void from_json(const json& j, activate& msg)
  {
    from_json(j, dynamic_cast<message&>(msg));
    j.at("reset_time").get_to(msg._reset_time);
    j.at("signal").get_to(msg._signal);
    j.at("enable_sigterm").get_to(msg._enable_sigterm);
    j.at("delay_after_sigterm").get_to(msg._delay_after_sigterm);
  }
} // namespace appdog::messages
