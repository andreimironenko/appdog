#include "client.h"
#include "client_.h"

namespace appdog
{

  client::client(pid_t pid, long tid) :
    _client(new client_{pid, tid})
  {}

  client::~client(){}

  int client::activate(long reset_time, int signal, bool enable_sigterm, long delay_after_sigterm)
  {
    return _client->activate(reset_time, signal, enable_sigterm, delay_after_sigterm);
  }

  int client::deactivate()
  {
    return _client->deactivate();
  }

  int client::kick()
  {
    return _client->kick();
  }
} // namespace appdog


