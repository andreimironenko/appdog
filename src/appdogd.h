//
// Created by amironenko on 13/11/2020.
//
#pragma once

// C++ STL headers
#include <memory>
#include <map>
#include <utility>
#include <chrono>
#include <ratio>
#include <functional>
#include <ostream>

// Linux headers
#include <signal.h>

// Posix headers

// Boost headers
#include <boost/program_options.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

// open-source libraries
#include "nlohmann/json.hpp"

// local headers
#include "appdog.h"
#include "messages/message.h"
#include "timer.h"

namespace po = boost::program_options;
using namespace std::chrono;


namespace appdog {

  class appdogd;


  appdogd& get_appdogd(int argc = 0, char** argv = nullptr);

  using signal_handler_t = void (*) (int);
  using client_key_t = std::pair<pid_t, long>;

  class appdogd : public std::enable_shared_from_this<appdogd> {
    class client;

    friend appdogd& get_appdogd(int argc, char** argv);
    friend std::ostream& operator << (std::ostream &out, const appdogd::client& cl);

    class client {
      friend std::ostream& operator << (std::ostream &out, const appdogd::client& cl);

      appdogd* _appdogd;
      pid_t _pid;
      long _tid;
      duration<long, std::nano> _reset_time;
      std::unique_ptr<ipc::message_queue> _queue;
      timer::callback_t _timer_handler;
      timer::callback_t _sigterm_timer_handler;
      int  _sig;
      duration<long, std::nano> _delay_after_sigterm;
      std::unique_ptr<timer> _timer;
      std::unique_ptr<timer> _sigterm_timer;

      std::string queue_name() const;

      client() = default;

      friend void swap(client& first, client& second) noexcept
      {
        std::swap(first._appdogd, second._appdogd);

        //std::swap((long)first._pid, (long)second._pid);
        pid_t tmp_pid{first._pid};
        first._pid = second._pid;
        second._pid = tmp_pid;

        std::swap(first._tid, second._tid);
        std::swap(first._sig, second._sig);

        //std::swap(first._reset_time, second._reset_time);
        duration<long, std::nano> tmp_reset_time(first._reset_time);
        first._reset_time = second._reset_time;
        second._reset_time = tmp_reset_time;

        //std::swap(first._queue, second._queue);
        first._queue.swap(second._queue);

        std::swap(first._timer_handler, second._timer_handler);
        std::swap(first._sigterm_timer_handler, second._sigterm_timer_handler);

        //std::swap(first._delay_after_sigterm, second._delay_after_sigterm);
        duration<long, std::nano> tmp_sigterm_time(first._delay_after_sigterm);
        first._delay_after_sigterm = second._delay_after_sigterm;
        second._delay_after_sigterm = tmp_sigterm_time;


        //std::swap(first._timer, second._timer);
        first._timer.swap(second._timer);

        //std::swap(first._sigterm_timer, second._sigterm_timer);
        first._sigterm_timer.swap(second._sigterm_timer);
      }

      public:
      explicit client(appdogd* appd,
          pid_t pid, long tid,
          duration<long, std::nano> reset_time,
          timer::callback_t timer_handler, timer::callback_t sigterm_timer_handler,
          int sig = SIGTERM, duration<long, std::nano> delay_after_sigterm = 0s);

      client(const client&) = delete;
      client(client&& rhs) = default;
      client& operator=(const client&) = delete;
      client& operator=(client&& rhs) = default;
      ~client();

      void activate();
      void deactivate();
      void kick();

      pid_t pid() {return _pid;}
      long tid() {return _tid;}
      int signal() {return _sig;}
      timer* get_timer() {return _timer.get();}

      void send(const char* data, size_t size);
    }; // class client

    friend client;

    static const long FD_OPEN_MAX = 8192;

    int _argc;
    char** _argv;

    std::string _work_directory;
    std::string _config_file;
    po::options_description _desc;
    pid_t _pid;

    int _log_options;
    int _log_facility;
    int _log_level;

    std::map<int, signal_handler_t> _signals;
    std::unique_ptr<ipc::message_queue> _mq_rx;
    std::unique_ptr<char[]> _rx_buffer;
    std::map<client_key_t, appdogd::client> _clients;
    std::map<messages::msg_id, void (appdogd::*)(const json&)> _actions;

    void parse_cli_options();
    void daemonize();
    void set_signals();

    void send_confirmation(const client_key_t &client_key);

    // actions
    void activate(const json &j);
    void deactivate(const json &j);
    void kick(const json &j);
    void clients(const json &j);
    void reload(const json &j);
    void stop(const json &j);
    void confirm(const json &j);

    explicit appdogd(int argc, char** argv);

    // timer handlers
    void timer_handler(void* client_ptr);
    void sigterm_timer_handler(void* client_ptr);

    // send message to client with (pid, tid)
    void send(pid_t pid, long tid, const char* data, size_t size);
    void send_cnf_unknow_client(pid_t pid, long tid);

    public:

    appdogd(const appdogd&) = delete;
    appdogd(appdogd&&) = delete;
    appdogd& operator=(const appdogd&) = delete;
    appdogd& operator=(appdogd&&) = delete;

    ~appdogd();


    std::string work_directory() const;
    void set_work_directory(std::string work_directory);

    std::string config_file() const;
    void set_config_file(std::string config_file);
    int log_level() const; void set_log_level(int level); [[noreturn]] void run();
  }; //class appdogd

  std::ostream& operator << (std::ostream &out, const appdogd::client& cl);
} // namespace appdog
