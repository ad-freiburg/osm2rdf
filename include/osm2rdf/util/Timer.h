// Copyright 2011-2023, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Johannes Kalmbach <kalmbach@cs.uni-freiburg.de>
#pragma once

#include <sys/time.h>
#include <sys/types.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

namespace ad_utility {
using namespace std::string_literals;
using namespace std::chrono_literals;

// This internal namespace is used such that we can use an alias for the rather
// long `std::chrono` namespace without leaking this alias into `ad_utility`.
// All the defined types are exported into the `ad_utility` namespace via
// `using` declarations.
namespace timer {
namespace chr = std::chrono;

// A simple class for time measurement.
class Timer {
 public:
  // Typedefs for some types from `std::chrono`. milli- and microseconds are
  // stored as integrals and seconds are stored as doubles. The timer internally
  // works with microseconds.
  using Microseconds = chr::microseconds;
  using Milliseconds = chr::milliseconds;
  using Seconds = chr::duration<double>;
  using Duration = Microseconds;
  using TimePoint = chr::time_point<chr::high_resolution_clock>;

  // A simple enum used in the constructor to decide whether the timer is
  // immediately started or not.
  enum class InitialStatus { Started, Stopped };
  // Allow the usage of `Timer::Started` and `Timer::Stopped`.
  // TODO<joka921, GCC 12.3> This could be `using enum InitialStatus`,
  // but that leads to an internal compiler error in GCC. I suspect that it is
  // this bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=103081
  static constexpr InitialStatus Started = InitialStatus::Started;
  static constexpr InitialStatus Stopped = InitialStatus::Stopped;

  // Convert any `std::chrono::duration` to the underlying `Duration` type
  // of the `Timer` class.
  template <typename T>
  static Duration toDuration(T duration) {
    return chr::duration_cast<Duration>(duration);
  }

  // Convert a `Duration` to seconds (as a plain `double`).
  // TODO<joka921> As soon as we have `std::format` or something similar that
  // allows for simple formatting of `std::chrono` types, these functions
  // also should return `std::chrono::duration` for more typesafety.
  static double toSeconds(Duration d) {
    return chr::duration_cast<Seconds>(d).count();
  }

  // Convert a `Duration` to milliseconds (as a plain `size_t`).
  static size_t toMilliseconds(Duration d) {
    return chr::duration_cast<Milliseconds>(d).count();
  }

 private:
  // The timer value (initially zero)
  Duration value_ = Duration::zero();
  TimePoint timeOfStart_;
  bool isRunning_ = false;

 public:
  Timer(InitialStatus initialStatus = InitialStatus::Started) {
    if (initialStatus == Started) {
      start();
    }
  }

  // Reset the timer value to zero and stops the measurement.
  void reset() {
    value_ = Duration::zero();
    isRunning_ = false;
  }

  // Reset the timer value to zero and starts the measurement.
  inline void start() {
    value_ = Duration::zero();
    timeOfStart_ = chr::high_resolution_clock::now();
    isRunning_ = true;
  }

  // Continue the measurement without resetting
  // the timer value (no effect if running)
  inline void cont() {
    if (isRunning_ == false) {
      timeOfStart_ = chr::high_resolution_clock::now();
      isRunning_ = true;
    }
  }

  // Stop the measurement.
  inline void stop() {
    if (isRunning_) {
      value_ += timeSinceLastStart();
      isRunning_ = false;
    }
  }
  // The following functions return the current time of the timer.
  // Note that this also works while the timer is running.
  Duration value() const {
    if (isRunning()) {
      return value_ + timeSinceLastStart();
    } else {
      return value_;
    }
  }

  size_t msecs() const { return toMilliseconds(value()); }
  double secs() const {return toSeconds(value());}

  // is the timer currently running
  bool isRunning() const { return isRunning_; }

 private:
  Duration timeSinceLastStart() const {
    auto now = chr::high_resolution_clock::now();
    return toDuration(now - timeOfStart_);
  }
};

}  // namespace timer
using timer::Timer;
} // namespace ad_utility