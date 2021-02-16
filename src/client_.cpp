#include "client_.h"
#include "client.h"
#include "appdog.h"
#include "messages/activate.h"
#include "messages/confirm.h"
#include "messages/deactivate.h"
#include "messages/kick.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace appdog
{
  void client::client_::get_confirm_msg()
  {
    ipc::message_queue::size_type received_size;
    unsigned int priority = 0;

    _mq_rx->receive(_rx_buffer.get(), MQ_MAX_MSG_SIZE, received_size, priority);

    if (received_size == 0)
    {
      throw std::system_error(appdog::make_error_code(error::confmsg_zero_size));
    }

    auto cnf_msg = std::string(_rx_buffer.get(), received_size);
    auto j = json::parse(cnf_msg);
    messages::confirm cnf = j;

    if (cnf._error_category == "appdog")
    {
      if (cnf._error < 0)
      {
        throw std::system_error(appdog::make_error_code(static_cast<appdog::error>(cnf._error)));
      }
    }
    else
    {
      if (cnf._error != 0)
      {
        throw std::system_error(std::make_error_code(static_cast<std::errc>(cnf._error)));
      }
    }
  }

  client::client_::client_(pid_t pid, long tid=0):
    _pid(pid),
    _tid(tid),
    _appdog_mq(new ipc::message_queue(ipc::open_only, mq_rx)),
    _mq_rx(new ipc::message_queue(ipc::create_only,
          (std::to_string(_pid) + "_" + std::to_string(_tid)).c_str(), MQ_MAX_NUM_MSG, MQ_MAX_MSG_SIZE)),
    _rx_buffer(new char[MQ_MAX_MSG_SIZE])
  {}

  client::client_::~client_()
  {
    ipc::message_queue::remove((std::to_string(_pid) + "_" + std::to_string(_tid)).c_str());
  }

  pid_t client::client_::pid() {return _pid;}
  long client::client_::tid() {return _tid;}

  void client::client_::activate(duration<long, std::nano> reset_time, int sig, duration<long,
      std::nano> delay_after_sigterm)
  {
    auto msg = std::make_unique<messages::activate>(pid(), tid(),
        reset_time,
        sig,
        delay_after_sigterm);

    json json_object = *msg;
    auto json_text = json_object.dump();
    _appdog_mq->send(json_text.c_str(), json_text.size(), 0);

    get_confirm_msg();
  }

  void client::client_::deactivate()
  {
    auto msg = std::make_unique<messages::deactivate>(_pid, _tid);

    json json_object = *msg;
    auto json_text = json_object.dump();
    _appdog_mq->send(json_text.c_str(), json_text.size(), 0);

    get_confirm_msg();
  }

  void client::client_::kick()
  {
    auto msg = std::make_unique<messages::kick>(_pid, _tid);

    json json_object = *msg;
    auto json_text = json_object.dump();
    _appdog_mq->send(json_text.c_str(), json_text.size(), 0);

    get_confirm_msg();
  }
} // namespace appdog::client
