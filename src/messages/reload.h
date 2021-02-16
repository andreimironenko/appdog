#pragma once
#include <sys/types.h>

#include "messages/message.h"

namespace appdog::messages
{
  struct reload : public message
  {
    friend void to_json(json& j, const message& msg);
    friend void from_json(const json& j,message& m);
    friend bool operator==(const message &lhs, const message &rhs);

    reload(pid_t pid = 0, long tid = 0) :
      message(msg_id::RELOAD, pid, tid)
    {}

    friend bool operator==(const reload &lhs, const reload &rhs)
    {
      if (lhs._id != rhs._id || lhs._pid != rhs._pid || lhs._tid != rhs._tid)
      {
        return false;
      }
      return true;
    }
  };

#if 0
  void to_json(json& j, const reload& msg)
  {
    j = json{{"id", msg._id}, {"pid", msg._pid}, {"tid", msg._tid}};
  }

  void from_json(const json& j, reload& msg)
  {
    j.at("id").get_to(msg._id);
    j.at("pid").get_to(msg._pid);
    j.at("tid").get_to(msg._tid);
  }
#endif
}
