// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/DispatchQueue.h"

#include <pthread.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <utility>

#include "osm2ttl/util/Ram.h"

// ____________________________________________________________________________
osm2ttl::util::DispatchQueue::DispatchQueue(size_t threadCount,
                                            size_t queueFactor,
                                            const std::string& name)
  : _threads(threadCount), _name(name), _factor(queueFactor) {
  _quit = false;
  _die = false;
  for (size_t i = 0; i < _threads.size(); ++i) {
    _threads[i] = std::thread(&osm2ttl::util::DispatchQueue::handler, this);
    pthread_setname_np(_threads[i].native_handle(), _name.c_str());
  }
  if (_factor > 0) {
    limit(_threads.size() * _factor);
  }
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::limit() noexcept {
  limit(size());
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::limit(size_t maxSize) noexcept {
  // Allow atleast one entry in the queue -> single threaded.
  const size_t oldSize = _maxSize;
  _maxSize = std::max(_threads.size() * _factor, maxSize);
  if (_maxSize > oldSize) {
    _cvHasSpace.notify_all();
  }
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::checkFreeRam() noexcept {
  if (_factor > 0) {
    return;
  }
  if (_insertCount++ != 0) {
    return;
  }
  return;
  const int64_t dangerRam = 2 * osm2ttl::util::ram::GIGA;
  const int64_t lowRam = 3 * osm2ttl::util::ram::GIGA;
  const int64_t enoughRam = 4 * osm2ttl::util::ram::GIGA;
  int64_t freeRam = osm2ttl::util::ram::available();
  if (freeRam < 0) {
    // Invalid read -> don't change anything.
    return;
  }
  if (_maxSize == std::numeric_limits<size_t>::max() && freeRam < lowRam) {
    // Low ram, ... try to keep current usage.
    limit();
  }
  if (_maxSize != std::numeric_limits<size_t>::max()) {
    if (freeRam > enoughRam) {
      // We have enough ram again, use it before somebody else does.
      unlimit();
    } else if (freeRam < dangerRam) {
      // Dangerously low ram, try to half our queues to reduce memory usage
      // after workers have taken items.
      limit(_maxSize / 2);
    }
  }
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::unlimit() noexcept {
  limit(std::numeric_limits<size_t>::max());
  // Increased storage capacity -> inform all inserting calls.
  _cvHasSpace.notify_all();
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
  _cvHasData.notify_all();

  for (size_t i = 0; i < _threads.size(); ++i) {
    if (_threads[i].joinable()) {
      _threads[i].join();
    }
  }
}

// ____________________________________________________________________________
size_t osm2ttl::util::DispatchQueue::size() noexcept {
  std::lock_guard<std::mutex> lock{_mutex};
  return _queue.size();
}

// ____________________________________________________________________________
bool osm2ttl::util::DispatchQueue::empty() noexcept {
  std::lock_guard<std::mutex> lock{_mutex};
  return _queue.empty();
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::dispatch(
  const std::function<void(void)>& op) noexcept {
  constexpr const std::chrono::milliseconds maxWaitMs{10};
  while (size() >= _maxSize) {
    std::unique_lock<std::mutex> lock{_mutex};
    _cvHasSpace.wait_for(lock, maxWaitMs, [this]{
      return _queue.size() < _maxSize;
    });
  }
  std::lock_guard<std::mutex> lock{_mutex};
  _queue.push(std::move(op));
  _cvHasData.notify_one();
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::dispatch(std::function<void(void)>&& op)
noexcept {
  constexpr const std::chrono::milliseconds maxWaitMs{10};
  while (size() >= _maxSize) {
    std::unique_lock<std::mutex> lock{_mutex};
    _cvHasSpace.wait_for(lock, maxWaitMs, [this]{
      return _queue.size() < _maxSize;
    });
  }
  std::lock_guard<std::mutex> lock{_mutex};
  _queue.push(std::move(op));
  _cvHasData.notify_one();
}

// ____________________________________________________________________________
void osm2ttl::util::DispatchQueue::handler(void) noexcept {
  std::unique_lock<std::mutex> lock(_mutex);
  do {
    _cvHasData.wait(lock, [this]{
      return (!_queue.empty() || _quit || _die);
    });
    if (!_die && !_queue.empty()) {
      auto op = std::move(_queue.front());
      _queue.pop();
      lock.unlock();
      _cvHasSpace.notify_one();
      op();
      lock.lock();
    }
  } while (!_die && (!_quit || !_queue.empty()));
  lock.unlock();
}
