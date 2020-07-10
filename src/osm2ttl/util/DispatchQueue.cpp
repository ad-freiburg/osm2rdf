// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/DispatchQueue.h"

#include <utility>

// ____________________________________________________________________________
osm2ttl::util::DispatchQueue::DispatchQueue(size_t threadCount)
  : _threads(threadCount) {
  for (size_t i = 0; i < _threads.size(); ++i) {
    _threads[i] = std::thread(&osm2ttl::util::DispatchQueue::handler, this);
  }
}

// ____________________________________________________________________________
osm2ttl::util::DispatchQueue::~DispatchQueue() {
  // Signal end
  std::unique_lock<std::mutex> lock(_lock);
  _quit = true;
  lock.unlock();
  _conditionVariable.notify_all();

  // Wait for finish
  for (size_t i = 0; i < _threads.size(); ++i) {
    if (_threads[i].joinable()) {
      _threads[i].join();
    }
  }
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::dispatch(
  const std::function<void(void)>& op) {
  std::unique_lock<std::mutex> lock(_lock);
  _queue.push(op);
  lock.unlock();
  _conditionVariable.notify_one();
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::dispatch(std::function<void(void)>&& op) {
  std::unique_lock<std::mutex> lock(_lock);
  _queue.push(op);
  lock.unlock();
  _conditionVariable.notify_one();
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::handler(void) {
  std::unique_lock<std::mutex> lock(_lock);
  do {
    _conditionVariable.wait(lock, [this]{
      return (!_queue.empty() || _quit);
    });
    if (!_quit && !_queue.empty()) {
      auto op = std::move(_queue.front());
      _queue.pop();
      lock.unlock();
      op();
      lock.lock();
    }
  } while (!_quit);
}
