#pragma once
#include <sys/types.h>

#include "messages/message.h"

namespace appdog::messages
{
  struct kick : public message
  {
    friend void to_json(json& j, const kick& msg);
    friend void from_json(const json& j, kick& m);

#if 0
    explicit kick(pid_t pid, long tid = 0) :
      message(msg_id::KICK, pid, tid)
    {}
#endif

    friend bool operator==(const kick &lhs, const kick &rhs)
    {
      if (lhs._id != rhs._id || lhs._pid != rhs._pid || lhs._tid != rhs._tid)
      {
        return false;
      }
      return true;
    }
  };

  void to_json(json& j, const kick& msg)
  {
    j = json{{"id", msg_id::KICK}, {"pid", msg._pid}, {"tid", msg._tid}};
  }

  void from_json(const json& j, kick& msg)
  {
    j.at("id").get_to(msg._id);
    j.at("pid").get_to(msg._pid);
    j.at("tid").get_to(msg._tid);
  }

} // namespace appdog::kicks
