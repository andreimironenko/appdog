//
// Created by amironenko on 31/10/2020.
//
/**!
 * The Linux/Unix daemon baseclass

 */

// Linux headers
#include <sys/types.h>
#include <signal.h>

// STL headers
#include <map>
#include <string>
#include <stdexcept>
#include <system_error>

// Third party headers
#include "nlohmann/json.hpp"

// Boost header
#include <boost/interprocess/ipc/message_queue.hpp>

#pragma once

using json = nlohmann::json;
namespace ipc = boost::interprocess;

namespace appdog
{
  inline static const size_t MQ_MAX_MSG_SIZE = 512;
  inline static const size_t MQ_MAX_NUM_MSG = 10;
  inline const char* mq_rx = "appdog_mq_rx";
  inline const char* mq_tx = "appdog_mq_tx";
  inline const char* daemon_name = "appdog";

  enum class error : int
  {
    // critical errors, decrease negative number to add a new error
    confmsg_unknown_err_category = -6,
    confmsg_zero_size = -5,
    confmsg_wrong_tid = -4,
    confmsg_wrong_pid = -3,
    failed_create_client = -2,
    unknown_client = -1,

    // warnings, increase number to add a new warning
    already_activated = 1
  };

  struct error_category : std::error_category
  {
    const char* name() const noexcept override
    {
      return "appdog";
    }

    std::string message(int err) const override
    {
      static std::map<int, std::string> err2str =
      {
        {static_cast<int>(error::confmsg_unknown_err_category), "confirm message unknown error category"},
        {static_cast<int>(error::confmsg_zero_size), "zero size confirm message"},
        {static_cast<int>(error::confmsg_wrong_tid), "confirm message with wron tid"},
        {static_cast<int>(error::confmsg_wrong_pid), "confirm message with wrong pid"},
        {static_cast<int>(error::failed_create_client), "failed create client"},
        {static_cast<int>(error::unknown_client), "unknown client"},
        {static_cast<int>(error::already_activated), "client is already activated"}
      };

      if (!err2str.count(err))
      {
        std::runtime_error("appdog::error_category::message err is unknown");
      }

      return err2str[err];
    }

    static error_category& instance()
    {
      static error_category instance;
      return instance;
    }

  };


  inline std::error_code make_error_code(error err) noexcept
  {
    return std::error_code(static_cast<int>(err), error_category::instance());
  }

  inline bool is_error(std::error_code ec)
  {
    return (ec && ec.value() < 0);
  }
} // namespace appdog

namespace std
{
  template <>
    struct is_error_code_enum<::appdog::error> : true_type {};
} // namespace std
