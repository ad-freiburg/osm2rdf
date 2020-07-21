// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_QUEUE_H_
#define OSM2TTL_UTIL_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>

namespace osm2ttl {
namespace util {

template<typename T>
class Queue {
 public:
  Queue();
  explicit Queue(size_t maxSize);

  Queue(const Queue& rhs) = delete;
  Queue& operator=(const Queue& rhs) = delete;
  Queue(Queue&& rhs) = delete;
  Queue& operator=(Queue&& rhs) = delete;

  T pop();
  void push(T item);

  void limit(size_t maxSize);
  size_t size();
  bool empty();

 protected:
  std::queue<T> _queue;
  std::mutex _mutex;
  std::condition_variable _hasItem;
  std::condition_variable _hasSpace;
  size_t _maxSize;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_QUEUE_H_
