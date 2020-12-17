#pragma once
#include <sys/types.h>

#include <messages/message.h>

namespace appdog::messages
{
  struct deactivate : public message
  {
    friend void to_json(json& j, const deactivate& msg);
    friend void from_json(const json& j, deactivate& m);

#if 0
    explicit deactivate(pid_t pid, long tid = 0) :
      message(msg_id::DEACTIVATE, pid, tid)
    {}
#endif

    friend bool operator==(const deactivate &lhs, const deactivate &rhs)
    {
      if (lhs._id != rhs._id || lhs._pid != rhs._pid || lhs._tid != rhs._tid)
      {
        return false;
      }
      return true;
    }
  };

  void to_json(json& j, const deactivate& msg)
  {
    j = json{{"id", msg_id::DEACTIVATE}, {"pid", msg._pid}, {"tid", msg._tid}};
  }

  void from_json(const json& j, deactivate& msg)
  {
    j.at("id").get_to(msg._id);
    j.at("pid").get_to(msg._pid);
    j.at("tid").get_to(msg._tid);
  }
} //namespace appdog::deactivates
