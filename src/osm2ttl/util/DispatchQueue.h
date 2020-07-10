// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_DISPATCHQUEUE_H_
#define OSM2TTL_UTIL_DISPATCHQUEUE_H_

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace osm2ttl {
namespace util {

class DispatchQueue {
 public:
  explicit DispatchQueue(size_t threadCount);
  ~DispatchQueue();
  void dispatch(const std::function<void(void)>& op);
  void dispatch(std::function<void(void)>&& op);

  DispatchQueue(const DispatchQueue& rhs) = delete;
  DispatchQueue& operator=(const DispatchQueue& rhs) = delete;
  DispatchQueue(const DispatchQueue&& rhs) = delete;
  DispatchQueue& operator=(DispatchQueue&& rhs) = delete;
 protected:
  void handler(void);
  std::mutex _lock;
  std::vector<std::thread> _threads;
  std::queue<std::function<void(void)>> _queue;
  std::condition_variable _conditionVariable;
  bool _quit = false;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_DISPATCHQUEUE_H_
