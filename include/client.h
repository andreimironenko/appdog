//
// Created by amironenko on 31/10/2020.
//

//Linux headers
#include <sys/types.h>
#include <signal.h>

//C++ STL headers
#include <memory>

#pragma once

// Local headers
namespace appdog
{
  class client
  {
    class client_;

    std::unique_ptr<client_> _client;


    public:
    explicit client(pid_t pid, long tid = 0);
    client(const client&) = delete;
    client(client&&) = delete;
    client& operator=(const client&) = delete;
    client& operator=(client&&) = delete;
    ~client();

    int activate(long reset_time=60, int signal=SIGTERM, bool enable_sigterm=false, long delay_after_sigterm=0);
    int deactivate();
    int kick();
  };
} //namespace appdog
