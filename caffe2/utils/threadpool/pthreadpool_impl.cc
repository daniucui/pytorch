#include "caffe2/utils/threadpool/pthreadpool.h"
#include "caffe2/utils/threadpool/ThreadPool.h"


//
// External API
//

void legacy_pthreadpool_compute_1d(
    legacy_pthreadpool_t threadpool,
    legacy_pthreadpool_function_1d_t function,
    void* argument,
    size_t range) {
  if (threadpool == nullptr) {
    /* No thread pool provided: execute function sequentially on the calling
     * thread */
    for (size_t i = 0; i < range; i++) {
      function(argument, i);
    }
    return;
  }
  reinterpret_cast<caffe2::ThreadPool*>(threadpool)
      ->run(
          [function, argument](int threadId, size_t workId) {
            function(argument, workId);
          },
          range);
}

void legacy_pthreadpool_parallelize_1d(
    const legacy_pthreadpool_t threadpool,
    const legacy_pthreadpool_function_1d_t function,
    void* const argument,
    const size_t range,
    uint32_t) {
  legacy_pthreadpool_compute_1d(threadpool, function, argument, range);
}

size_t legacy_pthreadpool_get_threads_count(legacy_pthreadpool_t threadpool) {
  // The current fix only useful when XNNPACK calls legacy_pthreadpool_get_threads_count with nullptr.
  if (threadpool == nullptr) {
    return 1;
  }
  return reinterpret_cast<caffe2::ThreadPool*>(threadpool)->getNumThreads();
}

legacy_pthreadpool_t legacy_pthreadpool_create(size_t threads_count) {
  std::mutex thread_pool_creation_mutex_;
  std::lock_guard<std::mutex> guard(thread_pool_creation_mutex_);

  return reinterpret_cast<legacy_pthreadpool_t>(new caffe2::ThreadPool(threads_count));
}

void legacy_pthreadpool_destroy(legacy_pthreadpool_t pthreadpool) {
  if (pthreadpool) {
    caffe2::ThreadPool* threadpool =
        reinterpret_cast<caffe2::ThreadPool*>(pthreadpool);
    delete threadpool;
  }
}
