// Created by amironenko on 19/11/2020.
//
//Linux headers
#include <sys/types.h>

// C++ STL headers
#include <memory>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <csignal>

#include <gtest/gtest.h>

// Boost headers
#include <boost/interprocess/ipc/message_queue.hpp>
#include "nlohmann/json.hpp"


#include "appdog.h"
#include "client.h"

namespace fs = std::filesystem;
namespace ipc = boost::interprocess;
using json = nlohmann::json;
using namespace appdog;

static volatile std::sig_atomic_t received_signal = 0;

static void signal_handler(int sig)
{
  received_signal = sig;
}

class AppDogTest: public ::testing::Test {
  protected:
    std::string _name;

  public:
    AppDogTest():
      _name("")
  {
    // initialization;
  }

    void SetUp( ) override {
      // initialization or some code to run before each test
      std::signal(SIGUSR1, signal_handler);
      std::signal(SIGTERM, signal_handler);
    }

    void TearDown( ) override {
      // code to run after each test;
      // can be used instead of a destructor,
      // but exceptions can be handled in this function only
    }

    ~AppDogTest( )  override {
      // resources cleanup, no exceptions allowed
    }
};

TEST_F(AppDogTest, SignalTimeOut) {
  EXPECT_TRUE(true);
  auto test_client = std::unique_ptr<client>(new client(getpid()));
  size_t res = test_client->activate(5, SIGUSR1);
  EXPECT_GT(res, 0);
  sleep(10);
  EXPECT_EQ(received_signal, SIGUSR1);
}

TEST_F(AppDogTest, DeactAppDog) {
  EXPECT_TRUE(true);
  auto test_client = std::unique_ptr<client>(new client(getpid()));

  size_t res = test_client->activate(5, SIGUSR1);
  EXPECT_GT(res, 0);

  sleep(1);

  res = test_client->deactivate();
  EXPECT_GT(res, 0);

  received_signal = 0;

  sleep(5);

  EXPECT_EQ(received_signal, 0);
}

TEST_F(AppDogTest, KickAppDog) {
  EXPECT_TRUE(true);
  auto test_client = std::unique_ptr<client>(new client(getpid()));
  size_t res = test_client->activate(5, SIGUSR1);
  EXPECT_GT(res, 0);
  for (int i = 0; i < 3; ++i)
  {
    sleep(4.75);
    res = test_client->kick();
    EXPECT_GT(res, 0);
  }

  res = test_client->deactivate();
  EXPECT_GT(res, 0);

  received_signal = 0;

  sleep(5);

  EXPECT_EQ(received_signal, 0);

}


