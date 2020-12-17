#pragma once
#include <sys/types.h>

#include "messages/message.h"

namespace appdog::messages
{
  struct reload : public message
  {
    friend void to_json(json& j, const confirm& msg);
    friend void from_json(const json& j, confirm& m);

#if 0
    explicit confirm(pid_t pid, long tid = 0) :
      message(msg_id::CONFIRM, pid, tid)
    {}
#endif

    friend bool operator==(const reload &lhs, const reload &rhs)
    {
      if (lhs._id != rhs._id || lhs._pid != rhs._pid || lhs._tid != rhs._tid)
      {
        return false;
      }
      return true;
    }
  };

  void to_json(json& j, const reload& msg)
  {
    j = json{{"id", msg_id::RELOAD}, {"pid", msg._pid}, {"tid", msg._tid}};
  }

  void from_json(const json& j, reload& msg)
  {
    j.at("id").get_to(msg._id);
    j.at("pid").get_to(msg._pid);
    j.at("tid").get_to(msg._tid);
  }
}
