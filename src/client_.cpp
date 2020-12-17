#include "client_.h"
#include "client.h"
#include "appdog.h"
#include "messages/activate.h"
#include "messages/confirm.h"
#include "messages/deactivate.h"
#include "messages/kick.h"

#include <iostream>

namespace appdog
{
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

  int client::client_::activate(long reset_time, int sig, bool enable_sigterm, long delay_after_sigterm)
  {
    auto msg = std::make_unique<messages::activate>();
    msg->_pid = _pid;
    msg->_tid = _tid;
    msg->_reset_time = reset_time;
    msg->_signal = sig;
    msg->_enable_sigterm = enable_sigterm;
    msg->_delay_after_sigterm = delay_after_sigterm;

    json json_object = *msg;
    auto json_text = json_object.dump();
    _appdog_mq->send(json_text.c_str(), json_text.size(), 0);
    ipc::message_queue::size_type received_size;
    unsigned int priority;

    _mq_rx->receive(_rx_buffer.get(), MQ_MAX_MSG_SIZE, received_size, priority);

    return static_cast<int>(received_size);
  }

  int client::client_::deactivate()
  {
    auto msg = std::make_unique<messages::deactivate>();
    msg->_pid = _pid;
    msg->_tid = _tid;

    json json_object = *msg;
    auto json_text = json_object.dump();
    _appdog_mq->send(json_text.c_str(), json_text.size(), 0);

    ipc::message_queue::size_type received_size;
    unsigned int priority;

    _mq_rx->receive(_rx_buffer.get(), MQ_MAX_MSG_SIZE, received_size, priority);

    return static_cast<int>(received_size);
  }

  int client::client_::kick()
  {
    auto msg = std::make_unique<messages::kick>();
    msg->_pid = _pid;
    msg->_tid = _tid;

    json json_object = *msg;
    auto json_text = json_object.dump();
    _appdog_mq->send(json_text.c_str(), json_text.size(), 0);

    ipc::message_queue::size_type received_size;
    unsigned int priority;

    _mq_rx->receive(_rx_buffer.get(), MQ_MAX_MSG_SIZE, received_size, priority);

    return static_cast<int>(received_size);
  }
} // namespace appdog::client
