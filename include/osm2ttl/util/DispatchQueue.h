// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_DISPATCHQUEUE_H_
#define OSM2TTL_UTIL_DISPATCHQUEUE_H_

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "osm2ttl/util/Queue.h"

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

  DispatchQueue(const DispatchQueue& rhs) = delete;
  DispatchQueue& operator=(const DispatchQueue& rhs) = delete;
  DispatchQueue(DispatchQueue&& rhs) = delete;
  DispatchQueue& operator=(DispatchQueue&& rhs) = delete;

 protected:
  void handler(void) noexcept;
  std::vector<std::thread> _threads;
  osm2ttl::util::Queue<std::function<void(void)>> _queue;
  std::mutex _mutex;
  std::condition_variable _cv;
  std::string _name;
  bool _quit = false;
  bool _die = false;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_DISPATCHQUEUE_H_
