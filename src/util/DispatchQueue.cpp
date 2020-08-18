// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/DispatchQueue.h"

#include <pthread.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <utility>

#include "osm2ttl/util/Ram.h"
#include "osm2ttl/util/Queue.h"

// ____________________________________________________________________________
osm2ttl::util::DispatchQueue::DispatchQueue(size_t threadCount,
                                            size_t queueFactor,
                                            const std::string& name)
  : _threads(threadCount), _name(name) {
  _quit = false;
  _die = false;
  if (queueFactor > 0) {
    _queue.limit(threadCount * queueFactor);
  }
  for (size_t i = 0; i < _threads.size(); ++i) {
    _threads[i] = std::thread(&osm2ttl::util::DispatchQueue::handler, this);
    pthread_setname_np(_threads[i].native_handle(), _name.c_str());
  }
}

// ____________________________________________________________________________
osm2ttl::util::DispatchQueue::~DispatchQueue() {
  _die = true;
  quit();
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::quit() noexcept {
  std::unique_lock<std::mutex> lock(_mutex);
  _quit = true;
  lock.unlock();
  _cv.notify_all();

  for (size_t i = 0; i < _threads.size(); ++i) {
    if (_threads[i].joinable()) {
      _threads[i].join();
    }
  }
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::dispatch(
  const std::function<void(void)>& op) noexcept {
  _queue.push(std::move(op));
  _cv.notify_one();
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::dispatch(std::function<void(void)>&& op)
noexcept {
  _queue.push(std::move(op));
  _cv.notify_one();
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::handler(void) noexcept {
  std::unique_lock<std::mutex> lock(_mutex);
  do {
    _cv.wait(lock, [this]{
      return (!_queue.empty() || _quit || _die);
    });
    if (!_queue.empty()) {
      auto op = std::move(_queue.pop());
      lock.unlock();
      op();
      lock.lock();
    }
  } while (!_die && (!_quit || !_queue.empty()));
}
