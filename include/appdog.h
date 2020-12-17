//
// Created by amironenko on 31/10/2020.
//
/**!
 * The Linux/Unix daemon baseclass

 */

//Linux headers
#include <sys/types.h>
#include <signal.h>

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
} //namespace appdog

