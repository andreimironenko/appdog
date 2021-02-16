#pragma once
#include <sys/types.h>
#include <string>
#include <cstring>

#include "messages/message.h"

namespace appdog::messages
{
  struct confirm : message
  {
    friend void to_json(json& j, const confirm& msg);
    friend void from_json(const json& j, confirm& m);

    int _error;
    std::string _error_category;
    std::string _error_msg;

    confirm(pid_t pid = 0, long tid = 0, const std::error_code& ec ={}) :
      message(msg_id::CONFIRM, pid, tid),
      _error(ec.value()),
      _error_category(ec.category().name()),
      _error_msg(ec.message())
    {}

    friend bool operator==(const confirm &lhs, const confirm &rhs)
    {
      if (lhs._id != rhs._id || lhs._pid != rhs._pid || lhs._tid != rhs._tid ||
          lhs._error != rhs._error || lhs._error_category == rhs._error_category)
      {
        return false;
      }
      return true;
    }
  };

  void to_json(json& j, const confirm& msg)
  {
    j = json {
      {"id", msg._id},
      {"pid", msg._pid}, {"tid", msg._tid},
      {"error", msg._error},
      {"error_category", msg._error_category},
      {"error_msg", msg._error_msg}
    };
  }

  void from_json(const json& j, confirm& msg)
  {
    j.at("id").get_to(msg._id);
    j.at("pid").get_to(msg._pid);
    j.at("tid").get_to(msg._tid);
    j.at("error").get_to(msg._error);
    j.at("error_category").get_to(msg._error_category);
    j.at("error_msg").get_to(msg._error_msg);
  }
} // namespace appdog::messages
