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
  DispatchQueue(size_t threadCount, size_t queueFactor,
                const std::string& name);
  ~DispatchQueue();
  void dispatch(const std::function<void(void)>& op) noexcept;
  void dispatch(std::function<void(void)>&& op) noexcept;
  void quit() noexcept;

  void limit() noexcept;
  void limit(size_t maxSize) noexcept;
  void unlimit() noexcept;
  void checkFreeRam() noexcept;

  size_t size() noexcept;
  bool empty() noexcept;

  DispatchQueue(const DispatchQueue& rhs) = delete;
  DispatchQueue& operator=(const DispatchQueue& rhs) = delete;
  DispatchQueue(const DispatchQueue&& rhs) = delete;
  DispatchQueue& operator=(DispatchQueue&& rhs) = delete;

 protected:
  void handler(void) noexcept;
  std::vector<std::thread> _threads;
  std::queue<std::function<void(void)>> _queue;
  std::mutex _mutex;
  std::condition_variable _cvHasData;
  std::condition_variable _cvHasSpace;
  std::string _name;
  size_t _maxSize = std::numeric_limits<size_t>::max();
  size_t _factor;
  uint8_t _insertCount = 0;
  bool _quit = false;
  bool _die = false;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_DISPATCHQUEUE_H_
