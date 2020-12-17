//Linux headers
#include <sys/types.h>
#include <signal.h>

// Third party headers
#include "nlohmann/json.hpp"

// Boost headers
#include <boost/interprocess/ipc/message_queue.hpp>

// Local headers
#include "client.h"

#pragma once

using json = nlohmann::json;
namespace ipc = boost::interprocess;

namespace appdog
{
  class client::client_
  {
    pid_t _pid = 0;
    long _tid = 0;
    std::unique_ptr<ipc::message_queue> _appdog_mq;
    std::unique_ptr<ipc::message_queue> _mq_rx;
    std::unique_ptr<char[]> _rx_buffer;

    public:
    explicit client_(pid_t pid, long tid);
    ~client_();

    client_(const client_&) = delete;
    client_(client_&&) = delete;
    client_& operator=(const client_&) = delete;
    client_& operator=(client_&&) = delete;

    pid_t pid();
    long tid();

    int activate(long reset_time, int signal, bool enable_sigterm, long delay_after_sigterm);
    int deactivate();
    int kick();
  };
}
