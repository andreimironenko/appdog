#include "client.h"
#include "client_.h"

#include <boost/python.hpp>
using namespace boost::python;

namespace appdog
{
  client::client(pid_t pid, long tid) :
    _client(std::make_shared<client_>(pid, tid))
  {

  }

  client::client(const client& src)
  {
    _client = src._client;
  }

  client::client(client&& src) noexcept
  {
    std::swap(_client, src._client);
    src._client.reset();
  }

  client& client::operator=(const client& rhs)
  {
    if (this == &rhs)
      return *this;

    _client = rhs._client;
    return *this;
  }

  client& client::operator=(client&& rhs) noexcept
  {

    std::swap(_client, rhs._client);
    rhs._client.reset();

    return *this;
  }


  client::~client(){}

  void client::activate(duration<long, std::nano> reset_time, int signal,
      duration<long, std::nano> delay_after_sigterm)
  {
    return _client->activate(reset_time, signal, delay_after_sigterm);
  }

  void client::activateL(long reset_time_ns, int signal, long delay_after_sigterm_ns)
  {
    return _client->activate(duration<long, std::nano>(reset_time_ns),
        signal, duration<long, std::nano>(delay_after_sigterm_ns));
  }

  void client::deactivate()
  {
    return _client->deactivate();
  }

  void client::kick()
  {
    return _client->kick();
  }

  BOOST_PYTHON_MODULE(libappdog)
  {
    class_<client>("client", init<pid_t, long>())
      .def("activateL", &client::activateL)
      .def("deactivate", &client::deactivate)
      .def("kick", &client::kick)
      ;
  }
} // namespace appdog



