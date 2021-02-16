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
#include <sstream>
#include <utility>
#include <sstream>
#include <stdexcept>
#include <cerrno>

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
  
  std::ostream& operator << (std::ostream &out, const appdogd::client& cl)
  {
    out << "_appdogd = 0x" << std::hex << (unsigned long) cl._appdogd << ", ";
    out << "_pid = " << std::dec << (long) cl._pid << ", ";
    out << "_tid = " << std::dec << cl._tid << ", ";
    out << "_reset_time = " << cl._reset_time.count() << " nsec, ";

    if (cl._timer_handler != nullptr)
      out << "_timer_handler = 0x" << std::hex << (unsigned long) cl._timer_handler.target<void(void*)>() << ", ";

    if (cl._sigterm_timer_handler != nullptr) {
      out << "_sigterm_timer_handler = 0x" << std::hex <<
        (unsigned long) cl._sigterm_timer_handler.target<void(void*)>() << ", ";
    }

    out << "_sig = " << std::dec << cl._sig << ", ";
    out << "_delay_after_sigterm = " << cl._delay_after_sigterm.count() << " nsec, ";

    if (cl._timer)
      out << "_timer = " << std::hex << (unsigned long) cl._timer.get() << ", ";
    if (cl._timer)
      out << "_sigterm_timer = " << std::hex << (unsigned long) cl._timer.get() << ", ";

    return out;
  }

  appdogd::client::client(
      appdogd* appdg,
      pid_t pid, long tid,
      duration<long, std::nano> reset_time,
      timer::callback_t timer_handler,
      timer::callback_t sigterm_timer_handler,
      int sig, duration<long, std::nano> delay_after_sigterm):
    _appdogd(appdg),
    _pid(pid),
    _tid(tid),
    _reset_time(reset_time),
    _queue(std::make_unique<ipc::message_queue>(ipc::open_only, (queue_name().c_str()))),
    _timer_handler(timer_handler),
    _sigterm_timer_handler(sigterm_timer_handler),
    _sig(sig),
    _delay_after_sigterm(delay_after_sigterm),
    _timer(nullptr),
    _sigterm_timer(nullptr)
  {
    //syslog(LOG_INFO, "appdogd::client this pointer: 0x%0x", (unsigned long)(this));
    _timer = std::unique_ptr<timer>(
        new timer (
          reset_time + delay_after_sigterm,
          std::bind(&appdogd::timer_handler, _appdogd, std::placeholders::_1),
          static_cast<void*>(this),
          true // single shot
          )
        );

    syslog(LOG_INFO, "client %ld: %ld with timer 0x%0lx", (unsigned long)(_pid), (unsigned long)_tid,
        (unsigned long)_timer.get());

    if(_delay_after_sigterm != 0s)
    {
      _sigterm_timer = std::unique_ptr<timer>(
          new timer(
            reset_time,
            std::bind(&appdogd::sigterm_timer_handler, _appdogd, std::placeholders::_1),
            static_cast<void*>(this),
            true // single shot
            )
          );
    }

    std::stringstream ss;
    ss << *this;
    syslog(LOG_INFO, "client %ld:%ld : %s", (unsigned long)(_pid), (unsigned long)_tid, ss.str().c_str());
  }

  appdogd::client::~client()
  {
    syslog(LOG_INFO, "client %ld: %ld dtor", (long)(_pid), _tid);
    ipc::message_queue::remove(queue_name().c_str());
  }

  std::string appdogd::client::queue_name() const
  {
    return std::string(std::to_string(_pid) + "_" + std::to_string(_tid));
  }

  void appdogd::client::activate()
  {
    syslog(LOG_INFO, "activate client %ld: %ld", (long)(_pid), _tid);

    _timer->start();

    if(_sigterm_timer)
    {
      syslog(LOG_INFO, "client %ld: %ld creating sigterm timer", (long)(_pid), _tid);
      _sigterm_timer->start();
    }

    syslog(LOG_INFO, "client %ld: %ld has activated", (long)(_pid), _tid);
  }

  void appdogd::client::deactivate()
  {
    syslog(LOG_INFO, "client::deactivate() for client %ld: %ld", static_cast<long>(_pid), _tid);

    if (_sigterm_timer)
    {
    syslog(LOG_INFO, "stopping sigterm_timer for the client %ld: %ld", static_cast<long>(_pid), _tid);
      _sigterm_timer->stop();
    }

    syslog(LOG_INFO, "stopping timer for the client %ld: %ld", static_cast<long>(_pid), _tid);
    //syslog(LOG_INFO, "_timer.get() = 0x%0lx", _timer.get());

    if(!_timer)
    {
      syslog(LOG_INFO, "_timer is nullptr");
      return;
    }
    _timer->stop();
  }

  void appdogd::client::kick()
  {
    if (_sigterm_timer)
    {
      _sigterm_timer->reset();
    }
    _timer->reset();
  }


  void appdogd::client::send(const char* data, size_t size)
  {
    syslog(LOG_INFO, "sendng data to the client %ld: %ld", static_cast<long>(_pid), _tid);
    _queue->send(data, size, 0);
  }

  // appdogd factory method
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

  // timer handlers
  void appdogd::timer_handler(void* client_ptr)
  {
    client* cl = static_cast<client*>(client_ptr);
    //syslog(LOG_INFO, "timer_handler: cl = 0x%0x", (unsigned long) cl);

    if (!cl)
    {
      syslog(LOG_ERR, "zero client pointer");
      return;
    }

    auto cl_pid = cl->pid();
    auto cl_tid = cl->tid();
    auto cl_key = client_key_t{cl_pid, cl_tid};

    std::stringstream ss;
    ss << *cl;
    syslog(LOG_INFO, "timer_handler: client %ld:%ld : %s", (unsigned long)(cl_pid), (unsigned long)cl_tid, ss.str().c_str());


    syslog(LOG_INFO, "sending signal %d to client %ld:%ld", cl->signal(), (long)(cl_pid), cl_tid);
    auto ret = kill(cl_pid, cl->signal());
    if (ret != 0)
    {
      syslog(LOG_ERR, "kill(%ld, %d) returns %d, %s", (long)cl_pid, cl->signal(), errno, strerror(errno));
      throw std::runtime_error("kill has failed");
    }

    syslog(LOG_INFO, "deactivating client %ld:%ld", (long)(cl_pid), cl_tid);
    cl->deactivate();

    syslog(LOG_INFO, "erasing client %ld:%ld", (long)(cl_pid), cl_tid);
    _clients.erase(cl_key);
  }

  void appdogd::sigterm_timer_handler(void* client_ptr)
  {
    auto cl = static_cast<client*>(client_ptr);

    if (!cl)
    {
      syslog(LOG_ERR, "zero client pointer");
      return;
    }
  }

  void appdogd::send(pid_t pid, long tid, const char* data, size_t size)
  {
    std::string queue_name(std::to_string(pid) + "_" + std::to_string(tid));
    auto client_queue = std::make_unique<ipc::message_queue>(ipc::open_only, (queue_name.c_str()));
    client_queue->send(data, size, 0);
  }

  void appdogd::send_cnf_unknow_client(pid_t pid, long tid)
  {
    messages::confirm cnf(pid, tid, appdog::make_error_code(error::unknown_client));
    json jcnf = cnf;
    auto text = jcnf.dump();
    send(pid, tid, text.c_str(), text.size());
  }

  void appdogd::activate(const json &j)
  {
    auto msg = j.get<messages::activate>();
    const auto& client_key = client_key_t(msg._pid, msg._tid);

    syslog(LOG_INFO, "appdogd::activate client %ld: %ld", static_cast<long>(msg._pid), msg._tid);

    //add_client just returns if the client already exists
    auto [iter, res] = _clients.try_emplace(
        client_key,
        std::move(
          *(new appdogd::client(
            this,
            msg._pid, msg._tid,
            duration<long, std::nano>(msg._reset_time),
            std::bind(&appdogd::timer_handler, this, std::placeholders::_1),
            std::bind(&appdogd::sigterm_timer_handler, this, std::placeholders::_1),
            msg._signal,
            duration<long, std::nano>(msg._delay_after_sigterm)
            ))
          )
        );

    if (res)
    {
      syslog(LOG_INFO, "sendng a confirmation back to the client %ld: %ld", static_cast<long>(msg._pid), msg._tid);
      auto& client = iter->second;
      client.activate();

      messages::confirm cnf(_pid, 0);
      json jcnf = cnf;
      auto text = jcnf.dump();

      client.send(text.c_str(), text.size());
    }
    else
    {
      syslog(LOG_ERR, "failed to create the client %ld: %ld", static_cast<long>(msg._pid), msg._tid);
      messages::confirm cnf(_pid, 0, appdog::make_error_code(error::failed_create_client));
      json jcnf = cnf;
      auto text = jcnf.dump();
      send(msg._pid, msg._tid, text.c_str(), text.size());
    }
  }

  void appdogd::deactivate(const json &j)
  {
    auto msg = j.get<messages::deactivate>();
    const auto& client_key = client_key_t(msg._pid, msg._tid);

    syslog(LOG_INFO, "client %ld: %ld deactivation", static_cast<long>(msg._pid), msg._tid);

    if(auto iter = _clients.find(client_key); iter != _clients.end())
    {
      auto& client = iter->second;
      client.deactivate();
      syslog(LOG_INFO, "sendng a confirmation back to the client %ld: %ld", static_cast<long>(msg._pid), msg._tid);
      messages::confirm cnf(_pid, 0);
      json jcnf = cnf;
      auto text = jcnf.dump();

      client.send(text.c_str(), text.size());
      syslog(LOG_INFO, "erasing client %ld:%ld", (long)(msg._pid), msg._tid);
      _clients.erase(client_key);
    }
    else
    {
      syslog(LOG_ERR, "client %ld: %ld is unknown", static_cast<long>(msg._pid), msg._tid);
      send_cnf_unknow_client(msg._pid, msg._tid);
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
      send_cnf_unknow_client(msg._pid, msg._tid);
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
