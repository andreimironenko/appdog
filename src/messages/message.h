#pragma once
#include <sys/types.h>

namespace appdog::messages
{
  enum class msg_id : int{
    ACTIVATE = 0,
    DEACTIVATE,
    KICK,
    CLIENTS,
    RELOAD,
    STOP,
    CONFIRM,
    MESSAGE_MAX
  };

  inline const char* msg_id_to_cstr[] = {
    "ACTIVATE",
    "DEACTIVATE",
    "KICK",
    "CLIENTS",
    "RELOAD",
    "STOP",
    "CONFIRM"
  };

  struct message
  {
    friend void to_json(json& j, const message& msg);
    friend void from_json(const json& j, message& m);

    msg_id _id;
    pid_t _pid;
    long _tid;

#if 0
    explicit message(msg_id id, pid_t pid, long tid = 0) :
      _id(id),
      _pid(pid),
      _tid(tid)
    {}
#endif

    friend bool operator==(const message &lhs, const message &rhs)
    {
      if (lhs._id != rhs._id || lhs._pid != rhs._pid || lhs._tid != rhs._tid)
      {
        return false;
      }
      return true;
    }
  };

  void to_json(json& j, const message& msg)
  {
    j = json{{"id", msg._id}, {"pid", msg._pid}, {"tid", msg._tid}};
  }

  void from_json(const json& j, message& msg)
  {
    j.at("id").get_to(msg._id);
    j.at("pid").get_to(msg._pid);
    j.at("tid").get_to(msg._tid);
  }
} //namespace appdog::messages
