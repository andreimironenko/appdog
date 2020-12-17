//
// Created by amironenko on 13/11/2020.
//

// Local headers
#include "appdogd.h"
#include "messages/activate.h"
#include "messages/confirm.h"
#include "messages/deactivate.h"
#include "messages/kick.h"
#include "messages/clients.h"
#include "messages/reload.h"

// STL C++ headers
#include <iostream>
#include <csignal>
#include <string>
#include <exception>
#include <sstream>

// Boost C++ headers
#include <boost/stacktrace.hpp>
//#include <boost/exception/all.hpp>

// Linux headers
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>


using std::stringstream;
using namespace appdog::messages;

namespace appdog {

  appdogd::client::client(pid_t pid, long tid, long reset_time, int sig,
      bool enable_sigterm, long delay_after_sigterm):
    _pid(pid),
    _tid(tid),
    _reset_time(reset_time),
    _sig(sig),
    _enable_sigterm(enable_sigterm),
    _delay_after_sigterm(delay_after_sigterm),
    _queue(std::make_unique<ipc::message_queue>(ipc::open_only, (queue_name().c_str()))),
    _timer(nullptr),         // timer
    _sigterm_timer(nullptr)  // sigterm_timer
  {
  }

  appdogd::client::~client()
  {
    if (_timer)
      timer_delete(_timer);

    if (_sigterm_timer)
      timer_delete(_sigterm_timer);

    ipc::message_queue::remove(queue_name().c_str());
  }

  std::string appdogd::client::queue_name() const
  {
    return std::string(std::to_string(_pid) + "_" + std::to_string(_tid));
  }

  void appdogd::client::activate()
  {
    syslog(LOG_INFO, "activate client %ld: %ld", (long)(_pid), _tid);

    struct itimerspec ts;
    struct sigaction sa;
    struct sigevent sev;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGRTMAX, &sa, NULL) == -1)
    {
      syslog(LOG_INFO, "call of sigaction(SIGRTMAX) is failed");
      return;
    }

    /* Create and start one timer for each command-line argument */
    sev.sigev_notify = SIGEV_SIGNAL;    /* Notify via signal */
    sev.sigev_signo = SIGRTMAX;         /* Notify using this signal*/
    sev.sigev_value.sival_ptr = &_timer;

    ts.it_value.tv_sec = _enable_sigterm ? _reset_time + _delay_after_sigterm : _reset_time;
    ts.it_value.tv_nsec = 0;

    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    /* Allows handler to get ID of this timer */
    syslog(LOG_INFO, "creating timer for client(%ld:%ld)", (long)_pid, (long)_tid);
    if (timer_create(CLOCK_REALTIME, &sev, &_timer) == -1)
    {
      syslog(LOG_ERR, "call of timer_create for client(%ld,%ld) is failed",
          (long)_pid, (long)_tid);
      return;
    }

    syslog(LOG_INFO, "firing up %ld sec timer 0x%0lx for client(%ld:%ld) ",
        _reset_time, (long)_timer, (long)_pid, (long)_tid);
    if (timer_settime(_timer, 0, &ts, NULL) == -1)
    {
      syslog(LOG_ERR, "call of timer_settime for client(%ld,%ld) is failed",
          (long)_pid, (long)_tid);
      return;
    }
    syslog(LOG_INFO, "activated %ld sec timer 0x%0lx for client(%ld:%ld) ",
        _reset_time, (long)_timer, (long)_pid, (long)_tid);


    if (_enable_sigterm)
    {
      // Enable SIGTERM timer
      sa.sa_flags = SA_SIGINFO;
      sa.sa_sigaction = sigterm_timer_handler;
      sigemptyset(&sa.sa_mask);

      if (sigaction(_sig, &sa, NULL) == -1)
      {
        syslog(LOG_INFO, "call of sigaction(SIGRTMAX) is failed");
        return;
      }

      /* Create and start one timer for each command-line argument */
      sev.sigev_notify = SIGEV_SIGNAL;    /* Notify via signal */
      sev.sigev_signo = SIGRTMAX;         /* Notify using this signal*/
      sev.sigev_value.sival_ptr =_timer;

      ts.it_value.tv_sec = 0;
      ts.it_value.tv_nsec = 0;

      ts.it_interval.tv_sec = _reset_time;
      ts.it_interval.tv_nsec = 0;

      /* Allows handler to get ID of this timer */
      syslog(LOG_INFO, "creating timer for client(%ld:%ld)", (long)_pid, (long)_tid);
      if (timer_create(CLOCK_REALTIME, &sev, &_sigterm_timer) == -1)
      {
        syslog(LOG_ERR, "call of timer_create for client(%ld,%ld) is failed",
            (long)_pid, (long)_tid);
        return;
      }

      if (timer_settime(_sigterm_timer, 0, &ts, NULL) == -1)
      {
        syslog(LOG_ERR, "call of timer_settime for client(%ld,%ld) is failed",
            (long)_pid, (long)_tid);
        return;
      }
    }
  }

  void appdogd::client::deactivate()
  {
    syslog(LOG_INFO, "killing timer for the client %ld: %ld", static_cast<long>(_pid), _tid);

    struct itimerspec ts{{0,0}, {0,0}};

    if (timer_settime(_timer, 0, &ts, NULL) == -1)
    {
      syslog(LOG_ERR, "call of timer_settings for client(%ld,%ld) is failed",
          (long)_pid, (long)_tid);
      return;
    }

    if (_enable_sigterm)
    {
      if (timer_settime(_sigterm_timer, 0, &ts, NULL) == -1)
      {
        syslog(LOG_ERR, "call of timer_settings for client(%ld,%ld) is failed",
            (long)_pid, (long)_tid);
        return;
      }
    }
  }

  void appdogd::client::kick()
  {
    deactivate();
    activate();
  }


  void appdogd::client::send(const char* data, size_t size)
  {
    syslog(LOG_INFO, "sendng data to the client %ld: %ld", static_cast<long>(_pid), _tid);
    _queue->send(data, size, 0);
  } 

  appdogd& get_appdogd(int argc, char** argv)
  {
    static appdogd appd{argc, argv};
    return appd;
  }


  appdogd::appdogd(int argc, char** argv):
    _argc(argc),
    _argv(argv),
    _desc("Available options"),
    _log_options(LOG_PID|LOG_CONS|LOG_NOWAIT),
    _log_facility(LOG_LOCAL0),
    _log_level(LOG_MASK(LOG_EMERG) | LOG_MASK(LOG_ALERT) | LOG_MASK(LOG_CRIT) | LOG_MASK(LOG_ERR)),
    _mq_rx(new ipc::message_queue(ipc::create_only, mq_rx, MQ_MAX_NUM_MSG, MQ_MAX_MSG_SIZE)),
    //_mq_rx(new ipc::message_queue(ipc::create_only, "appdogd", 10, 2048)),
    _rx_buffer(new char[MQ_MAX_MSG_SIZE]),
    _clients(),
    _actions {
      {msg_id::ACTIVATE, &appdogd::activate},
      {msg_id::DEACTIVATE, &appdogd::deactivate},
      {msg_id::KICK, &appdogd::kick},
      {msg_id::CLIENTS, &appdogd::clients},
      {msg_id::RELOAD, &appdogd::reload},
      {msg_id::STOP, &appdogd::stop},
      {msg_id::CONFIRM, &appdogd::confirm}
    }
  {}

  appdogd::~appdogd() {
    ipc::message_queue::remove(mq_rx);
  }

  void appdogd::parse_cli_options()
  {
    std::string work_directory;
    std::string config_file;
    _desc.add_options()
      ("help", " This help")
      ("work-directory", po::value<std::string>(&work_directory),
       "set daemon's working directory")
      ("config-file", po::value<std::string>(&config_file),
       "full path to configuration file")
      ;
    const char* dummy_option = "dummy";
    auto value = po::value<int>()->default_value(10);
    const char* dummy_option_description = "Just one of the dummy options";
    _desc.add_options()
      (dummy_option, value, dummy_option_description)
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(_argc, _argv, _desc), vm);
    po::notify(vm);

    if(!config_file.empty())
    {
      po::store(po::parse_config_file(config_file.c_str(), _desc), vm);
      po::notify(vm);
    }

    set_work_directory(work_directory);
    set_config_file(config_file);

    if (vm.count("help")) {
      std::cout << _desc << "\n";
    }
  }

  std::string appdogd::work_directory() const {return _work_directory;}
  void appdogd::set_work_directory(std::string work_directory) { _work_directory = work_directory;}

  std::string appdogd::config_file() const {return _config_file;}
  void appdogd::set_config_file(std::string config_file) {_config_file = config_file;}

  int appdogd::log_level() const {return _log_level;}
  void appdogd::set_log_level(int level) { _log_level = level;}


  void appdogd::daemonize()
  {
    // syslog initialsiation
    openlog(daemon_name, _log_options, _log_facility);

    _pid = fork();

    switch(_pid) // become a background process at first
    {
      case -1:                        // fork is failed
        _exit(EXIT_FAILURE);          // parent terminates
        break;
      case 0:                         // child falls through ...
        break;
      default:
        syslog(LOG_INFO, "Parent is out of business");
        _exit(EXIT_SUCCESS);          // parent terminates
    }

    syslog(LOG_INFO, "Become a leader of new session");
    if (-1 == setsid())
    {
      _exit(EXIT_SUCCESS);
    }

    // Second fork ensures that the daemon process is
    // not session leader
    _pid = fork();
    switch(_pid)
    {
      case -1:                      // fork is failed
        _exit(EXIT_FAILURE);        // parent terminates
        break;
      case 0:                       // child falls through ...
        syslog(LOG_INFO, "Starting daemon with PID");
        break;
      default:
        syslog(LOG_INFO, "Parent is out of business");
        _exit(EXIT_SUCCESS);        // parent terminates
    }

    // Clear file mode creation mask
    syslog(LOG_INFO, "Cleairing file mode creation mask");
    (void) umask(0);

    // Change to work directory
    syslog(LOG_INFO, "Changing to workdir");
    if (0 != chdir(_work_directory.c_str()))
    {
      syslog(LOG_INFO, "%s", strerror(errno));
    }

    // Close all open files
    syslog(LOG_INFO, "Closing all open files");
    long fd = 0;
    long maxfd = sysconf(_SC_OPEN_MAX);
    if (maxfd == -1)
      maxfd = FD_OPEN_MAX;

    for (long fd = 0; fd < maxfd; fd++)
    {
      close(fd);
    }

    // reopen standard fd's to /dev/null
    syslog(LOG_INFO, "Reopening standard fd's to /dev/null");
    close(STDIN_FILENO);

    fd = open("/dev/null", O_RDWR);
    if (fd != STDIN_FILENO)
    {
      _exit(EXIT_FAILURE);
    }

    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
    {
      _exit(EXIT_FAILURE);
    }

    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
    {
      _exit(EXIT_FAILURE);        // parent terminates
    }

    _pid = getpid();
    syslog(LOG_INFO, "Now daemon is fully functional, PID = %ld", (long)_pid);
  }

  void appdogd::set_signals()
  {
    // set sginal handlers
    if (!_signals.empty())
    {
      for (auto& [sig, handler] : _signals)
      {
        (void) std::signal(sig, handler);
      }
    }
  }

  appdogd::client* appdogd::find_client(timer_t timer)
  {
    for (auto& [k, c]: _clients)
    {
      if (c.timer() == timer)
      {
        return &c;
      }
    }
    return nullptr;
  }

  appdogd::client* appdogd::find_client_sigterm_timer(timer_t timer)
  {
    for (auto &[k, c]: _clients)
    {
      if (c.sigterm_timer() == timer)
      {
        return &c;
      }
    }
    return nullptr;
  }


  void appdogd::timer_handler(int sig, siginfo_t *si, void *uc = nullptr)
  {
    syslog(LOG_INFO, "timer_handler is called !!!!!!");
    timer_t timer_id = *(static_cast<timer_t*>(si->si_value.sival_ptr));
    auto& apdg = get_appdogd();
    auto cl = apdg.find_client(timer_id);
    auto cl_pid = cl->pid();
    auto cl_tid = cl->tid();
    auto cl_key = client_key_t{cl_pid, cl_tid};

    syslog(LOG_INFO, "time out for client %ld:%ld", (long)(cl_pid), cl_tid);
    if (!cl)
    {
      syslog(LOG_ERR, "unexpected timer 0x%08lx signal %d ", (unsigned long)(timer_id), sig);
      return;
    }

    syslog(LOG_INFO, "sending signal %d to client %ld:%ld", cl->signal(), (long)(cl_pid), cl_tid);
    auto ret = kill(cl_pid, cl->signal());

    if (ret != 0)
    {
      syslog(LOG_ERR, "kill(%ld, %d) returns %d, %s", (long)cl_pid, cl->signal(), errno, strerror(errno));
    }

    syslog(LOG_INFO, "deactivating client %ld:%ld", (long)(cl_pid), cl_tid);
    cl->deactivate();
    apdg._clients.erase(cl_key);
  }


  void appdogd::sigterm_timer_handler(int sig, siginfo_t *si, void *uc = nullptr)
  {

  }

  void appdogd::send_confirmation(const client_key_t &client_key)
  {
    auto [pid, tid] = client_key;
    syslog(LOG_INFO, "sendng a confirmation back to the client %ld: %ld", static_cast<long>(pid), tid);

    messages::confirm cnf;
    cnf._pid = pid;
    cnf._tid = tid;
    json jcnf = cnf;
    auto text = jcnf.dump();

    if(auto iter = _clients.find(client_key); iter != _clients.end())
    {
      iter->second.send(text.c_str(), text.size());
    }
  }


  void appdogd::activate(const json &j)
  {
    auto msg = j.get<messages::activate>();
    const auto& client_key = client_key_t(msg._pid, msg._tid);

    //add_client just returns if the client already exists
    auto [iter, res] = _clients.try_emplace(client_key, appdogd::client(msg._pid, msg._tid, msg._reset_time,
          msg._signal, msg._enable_sigterm, msg._delay_after_sigterm));

    if (res)
    {
      iter->second.activate();
    }

    syslog(LOG_INFO, "sendng a confirmation back to the client %ld: %ld", static_cast<long>(msg._pid), msg._tid);
    messages::confirm cnf;
    cnf._pid = _pid;
    cnf._tid = 0;
    json jcnf = cnf;
    auto text = jcnf.dump();

   iter->second.send(text.c_str(), text.size());
  }

  void appdogd::deactivate(const json &j)
  {
    auto msg = j.get<messages::deactivate>();
    const auto& client_key = client_key_t(msg._pid, msg._tid);

    syslog(LOG_INFO, "client %ld: %ld deactivation", static_cast<long>(msg._pid), msg._tid);

    if(auto iter = _clients.find(client_key); iter != _clients.end())
    {
      iter->second.deactivate();
      syslog(LOG_INFO, "sendng a confirmation back to the client %ld: %ld", static_cast<long>(msg._pid), msg._tid);
      messages::confirm cnf;
      cnf._pid = _pid;
      cnf._tid = 0;
      json jcnf = cnf;
      auto text = jcnf.dump();

      iter->second.send(text.c_str(), text.size());
    }
    else
    {
      syslog(LOG_ERR, "client %ld: %ld is unknown", static_cast<long>(msg._pid), msg._tid);
    }
  }


  void appdogd::kick(const json &j)
  {
    auto msg = j.get<messages::deactivate>();
    const auto& client_key = client_key_t(msg._pid, msg._tid);

    syslog(LOG_INFO, "client %ld: %ld kick signal is received", static_cast<long>(msg._pid), msg._tid);

    if(auto iter = _clients.find(client_key); iter != _clients.end())
    {
      iter->second.kick();
      syslog(LOG_INFO, "sendng a confirmation back to the client %ld: %ld", static_cast<long>(msg._pid), msg._tid);
      messages::confirm cnf;
      cnf._pid = _pid;
      cnf._tid = 0;
      json jcnf = cnf;
      auto text = jcnf.dump();

      iter->second.send(text.c_str(), text.size());
    }
    else
    {
      syslog(LOG_ERR, "client %ld: %ld is unknown", static_cast<long>(msg._pid), msg._tid);
    }

  }

  void appdogd::clients(const json &j)
  {

  }

  void appdogd::reload(const json &j)
  {

  }

  void appdogd::stop(const json &j)
  {

  }

  void appdogd::confirm(const json &j)
  {

  }




  [[noreturn]] void appdogd::run()
  {
    parse_cli_options();
    daemonize();
    set_signals();

    unsigned int priority = 0;
    ipc::message_queue::size_type received_size;

    syslog(LOG_INFO, "starting run-loop");

    while(true)
    {
      try {
      _mq_rx->receive(_rx_buffer.get(), MQ_MAX_MSG_SIZE, received_size, priority);

      syslog(LOG_INFO, "recvd_size %ld", received_size);
      auto msg = std::string(_rx_buffer.get(), received_size);
      syslog(LOG_INFO, "message: %s", msg.c_str());
        auto json_object = json::parse(msg);
        auto message_id = json_object["id"].get<messages::msg_id>();
        if (message_id >= msg_id::MESSAGE_MAX)
        {
          std::stringstream ss;
          ss <<"message id " << static_cast<int>(message_id) << ":";
          ss << "is out of range 0.." << static_cast<int>(msg_id::MESSAGE_MAX) << "\n";
          ss << "json: " << json_object.dump() << "\n";
          syslog(LOG_INFO, "logic_error: %s", ss.str().c_str());

          throw std::out_of_range(ss.str());
        }
        syslog(LOG_INFO, "message id = %d: %s ", static_cast<int>(message_id),
            msg_id_to_cstr[static_cast<int>(message_id)]);

        (this->*_actions[message_id])(json_object);
      }
      catch(const std::exception& e)
      {
        syslog(LOG_ERR, "%s", e.what());
      }
    }
  }

} // namespace appdog

int main(int argc, char** argv)
{
  syslog(LOG_INFO, "starting appdog daemon");

  syslog(LOG_INFO, "removing existing queue");
  ipc::message_queue::remove(appdog::mq_rx);

  try {
    auto& ad = appdog::get_appdogd(argc, argv);
    ad.run();
  }
  catch (const std::exception& expt)
  {
    syslog(LOG_INFO, "%s", expt.what());
    std::stringstream ss;
    ss << boost::stacktrace::stacktrace();
    //auto st = boost::get_error_info<traced>(e);
    //ss << st;
    syslog(LOG_INFO, "stacktrace: %s", ss.str().c_str());
  }


  return 0;
}
