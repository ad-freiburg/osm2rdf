// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/Queue.h"

#include <iostream>
#include <functional>
#include <limits>
#include <mutex>
#include <utility>

// ____________________________________________________________________________
template<typename T>
osm2ttl::util::Queue<T>::Queue() :
  _maxSize(std::numeric_limits<size_t>::max()) {
}

// ____________________________________________________________________________
template<typename T>
osm2ttl::util::Queue<T>::Queue(size_t maxSize) : _maxSize(maxSize) {}

// ____________________________________________________________________________
template<typename T>
T osm2ttl::util::Queue<T>::pop() {
  std::unique_lock<std::mutex> lock(_mutex);
  while (_queue.empty()) {
    _hasItem.wait(lock);
  }
  auto item = _queue.front();
  _queue.pop();
  _hasSpace.notify_one();
  return item;
}

// ____________________________________________________________________________
template<typename T>
void osm2ttl::util::Queue<T>::push(T item) {
  std::unique_lock<std::mutex> lock(_mutex);
  while (_queue.size() >= _maxSize) {
    _hasSpace.wait(lock);
  }
  _queue.push(std::move(item));
  lock.unlock();
  _hasItem.notify_one();
}

// ____________________________________________________________________________
template<typename T>
void osm2ttl::util::Queue<T>::limit(size_t maxSize) {
  std::unique_lock<std::mutex> lock(_mutex);
  const size_t oldSize = _maxSize;
  _maxSize = maxSize;
  if (oldSize < maxSize) {
    _hasSpace.notify_all();
  }
}

// ____________________________________________________________________________
template<typename T>
size_t osm2ttl::util::Queue<T>::size() {
  std::unique_lock<std::mutex> lock(_mutex);
  return _queue.size();
}

// ____________________________________________________________________________
template<typename T>
bool osm2ttl::util::Queue<T>::empty() {
  std::unique_lock<std::mutex> lock(_mutex);
  return _queue.empty();
}


// ____________________________________________________________________________
template class osm2ttl::util::Queue<std::function<void(void)>>;
