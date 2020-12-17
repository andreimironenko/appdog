#include "daemon.h"
#include <boost/program_options.hpp>

using rtcxx::daemon_base;

namespace po = boost::program_options;

int main(int argc, char** argv) {

  auto dm = std::make_unique<daemon_base>("simple_daemon", argc, argv);
  dm->set_log_level(LOG_UPTO(LOG_INFO));
  auto desc = dm->get_options_description();
  std::string prefix("my prefix");
  desc.add_options() ("prefix", po::value<int>()->default_value(10), "echo message prefix");
  dm->start();

  return 0;
}
