// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_DISPATCHQUEUE_H_
#define OSM2TTL_UTIL_DISPATCHQUEUE_H_

#include <condition_variable>
#include <functional>
#include <limits>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace osm2ttl {
namespace util {

class DispatchQueue {
 public:
  DispatchQueue(size_t threadCount, const std::string& name);
  ~DispatchQueue();
  void dispatch(const std::function<void(void)>& op);
  void dispatch(std::function<void(void)>&& op);
  void quit();

  void limit();
  void limit(size_t maxSize);
  void unlimit();
  void checkFreeRam();

  DispatchQueue(const DispatchQueue& rhs) = delete;
  DispatchQueue& operator=(const DispatchQueue& rhs) = delete;
  DispatchQueue(const DispatchQueue&& rhs) = delete;
  DispatchQueue& operator=(DispatchQueue&& rhs) = delete;

 protected:
  void handler(void);
  std::mutex _lockIn;
  std::mutex _lockOut;
  std::vector<std::thread> _threads;
  std::queue<std::function<void(void)>> _queue;
  std::condition_variable _conditionVariableIn;
  std::condition_variable _conditionVariableOut;
  std::string _queueName;
  size_t _maxSize = std::numeric_limits<size_t>::max();
  bool _quit = false;
  bool _die = false;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_DISPATCHQUEUE_H_
