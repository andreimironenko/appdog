//
// Created by amironenko on 13/11/2020.
//
#pragma once

// C++ STL headers
#include <memory>
#include <map>
#include <utility>

// Linux headers
#include <signal.h>
#include <time.h>

// Posix headers

// Boost headers
#include <boost/program_options.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

// open-source libraries
#include "nlohmann/json.hpp"

// local headers
#include "appdog.h"
#include "messages/message.h"

namespace po = boost::program_options;

namespace appdog {
  class appdogd;

  appdogd& get_appdogd(int argc = 0, char** argv = nullptr);

  using signal_handler_t = void (*) (int);
  using msg_handler_t = void (*) (size_t size, char*);
  using client_key_t = std::pair<pid_t, long>;

  class appdogd {
    friend appdogd& get_appdogd(int argc, char** argv);
    
    class client {
      pid_t _pid;
      long _tid;
      long _reset_time;
      int  _sig;
      bool _enable_sigterm;
      long _delay_after_sigterm;
      std::unique_ptr<ipc::message_queue> _queue;
      timer_t _timer;
      timer_t _sigterm_timer;

      std::string queue_name() const;

      
      public:
      //client() {throw std::logic_error("appdogd: attempt to allocate an empty client");}
      explicit client(pid_t pid, long tid, long reset_time, int sig, bool enable_sigterm,
          long delay_after_sigterm);
      client(const client&) = delete;
      client(client&&) = default; 
      client& operator=(const client&) = delete;
      client& operator=(client&&) = default;
      ~client();

      timer_t timer() {return _timer;};
      timer_t sigterm_timer() {return _sigterm_timer;};

      void activate();
      void deactivate();
      void kick();

      pid_t pid() {return _pid;}
      long tid() {return _tid;}
      int signal() {return _sig;}

      void send(const char* data, size_t size);
    };


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
    client* find_client(timer_t timer);
    client* find_client_sigterm_timer(timer_t timer);

    // Actions
    void activate(const json &j);
    void deactivate(const json &j);
    void kick(const json &j);
    void clients(const json &j);
    void reload(const json &j);
    void stop(const json &j);
    void confirm(const json &j);

    explicit appdogd(int argc, char** argv);

    public:

    static void timer_handler(int sig, siginfo_t *si, void *uc);
    static void sigterm_timer_handler(int sig, siginfo_t *si, void *uc);

    appdogd(const appdogd&) = delete;
    appdogd(appdogd&&) = delete;
    appdogd& operator=(const appdogd&) = delete;
    appdogd& operator=(appdogd&&) = delete;

    ~appdogd();


    std::string work_directory() const;
    void set_work_directory(std::string work_directory);

    std::string config_file() const;
    void set_config_file(std::string config_file);
    int log_level() const; void set_log_level(int level); [[noreturn]] void run(); }; } // namespace appdog
