//
// Created by amironenko on 31/10/2020.
//

//Linux headers
#include <sys/types.h>
#include <signal.h>

//C++ STL headers
#include <memory>
#include <ratio>
#include <chrono>
#include <utility>

#include "appdog.h"

#pragma once

//using std::chrono;
using std::chrono::duration;
using namespace std::chrono_literals;

// Local headers
namespace appdog
{
  class client
  {
    class client_;

    std::shared_ptr<client_> _client;

    public:
    explicit client(pid_t pid, long tid = 0);
    client(const client& rhs);
    client(client&&) noexcept;
    client& operator=(const client&);
    client& operator=(client&&) noexcept;
    ~client();

   void activate(duration<long, std::nano> reset_time=60s, int signal=SIGTERM,
        duration<long, std::nano> delay_after_sigterm=0ns);
   void activateL(long reset_time_ns, int signal, long delay_after_sigterm_ns);
   void deactivate();
   void kick();
  };

  } //namespace appdog


