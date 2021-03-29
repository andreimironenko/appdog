// C++ STL headers
#include <csignal>
#include <chrono>
#include <ratio>
#include <memory>
#include <functional>

#pragma once

/**
 * @brief A simple wrapper class around POSIX timer API.
 * This implementation has the following limitations:
 *  - not thread safe;
 *  - not sharable;
 *  - not copyable;
 *  - not movable;
 * This class implements PIMPL idiom, there the timer class which is defined in this header has forward declaration
 * for the implementation class timer_, so even if the implementation of timer_ has changed the exported API remains
 * the same and does not require recompling, but just updating with the new version of library.
 */
class timer {

  // forward declaration of the implementation class
  class timer_;
  /** Private pointer to the implementation class object. */
  std::unique_ptr<timer_> _timer;

  public:
  /**
   * This is std::function type which holds a pointer to user defined callback funtion.
   * This  function is called when the timer expires.
   */
  using callback_t = std::function<void(void*)>;


  /**
   * Public class timer constructor.
   * The shortest form for call would be
   *    auto t = timer(5s),
   * where 5s is literal type defined in std::chrono.
   * @param period_nsec
   */
  explicit timer(std::chrono::duration<long, std::nano> period_nsec,
      callback_t callback = nullptr, void* data = nullptr,
      bool is_single_shot = false, int sig = SIGRTMAX
      );

  ~timer();

  timer(const timer&) = delete;
  timer(timer&&) = delete;
  timer& operator=(const timer&) = delete;
  timer& operator=(timer&&) = delete;

  void start();
  void reset();
  void suspend();
  void resume();
  void stop();
};
