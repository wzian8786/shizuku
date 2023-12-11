#include "szk_dispatch.h"
#include <cstdlib>
namespace util {
const char* Dispatcher::kThreadNumEnv = "SHIZUKU_THREAD_NUM";
Dispatcher Dispatcher::gDispatcher(Dispatcher::kDefaultThreadNum);
Dispatcher::Dispatcher(size_t threadNum) :
                    _status(kIdle),
                    _tasks(0),
                    _threadPool(1) {
}
Dispatcher::~Dispatcher()  {
    _status.store(kStop);
    unlock();
    for (size_t i = 0; i < _threadPool.size(); ++i) {
        pthread_join(_threadPool[i].pid, nullptr);
    }
}

void Dispatcher::lock(size_t end, size_t begin) {
    end = end ? end : _threadPool.size();
    Assert(begin);
    for (size_t i = begin; i < end; ++i) {
        pthread_mutex_lock(&_threadPool[i].lock);
    }
}

void Dispatcher::unlock(size_t end, size_t begin) {
    end = end ? end : _threadPool.size();
    Assert(begin);
    for (size_t i = begin; i < end; ++i) {
        pthread_mutex_unlock(&_threadPool[i].lock);
    }
}

size_t Dispatcher::determinThreadNum(size_t threads, const Range& range) {
    const char* env = getenv(kThreadNumEnv);
    if (env && (*env) && (threads != 1)) {
        threads = strtoul(env, nullptr, 0);
    }
    threads = threads ? threads : kDefaultThreadNum;
    size_t begin = range.first;
    size_t end = range.second;
    Assert(end > begin);
    size_t tasks = end - begin;
    threads = (tasks >= threads * Wsqueue::kReservedTasks) ?
                        threads : (tasks / Wsqueue::kReservedTasks);
    threads = threads ? threads : 1;
    return threads;
}

void Dispatcher::init(size_t threads) {
    size_t inited = _threadPool.size();
    if (threads > inited) {
        _threadPool.resize(threads);
        lock(threads, inited);
        for (size_t i = inited; i < threads; ++i) {
            Args* args = new Args{ this, i };
            pthread_create(&_threadPool[i].pid, nullptr, threadEntry, args);
        }
    }
}

void Dispatcher::run(size_t threads, const Range& range) {
    if (range.first == range.second) return;
    size_t begin = range.first;
    size_t end = range.second;
    Assert(end > begin);
    size_t tasks = end - begin;
    size_t step = tasks / threads;

    _tasks.store(tasks);
    for (size_t i = 0; i < threads; ++i) {
        uint32_t v1 = begin + i * step;
        uint32_t v2 = begin + (i + 1) * step;
        if ((i + 1) == threads) {
            v2 += (tasks % threads);
        }
        // 0 is reserverd in Wsqueue
        _threadPool[i].q.reset(v1+1, v2+1);
        _threadPool[i].q.addVictim(&_threadPool[(i+1) % threads].q);
        _threadPool[i].q.addVictim(&_threadPool[(i+2) % threads].q);
        _threadPool[i].q.addVictim(&_threadPool[(i+3) % threads].q);
    }

    unlock(threads);

    // make sure all the threads are running
    for (size_t i = 1; i < threads; ++i) {
        while (!_threadPool[i].ack) {}
    }
    lock(threads);
    _status.store(kRunning);

    _threadPool[0].wrapper->runQ(_threadPool[0].q);

    // make sure all the tasks are done
    for (size_t i = 1; i < threads; ++i) {
        while (_threadPool[i].ack) {}
    }

    for (size_t i = 1; i < threads; ++i) {
        _threadPool[0].wrapper->join(_threadPool[i].wrapper.get());
        _threadPool[i].wrapper = nullptr;
    }
    _threadPool[0].wrapper = nullptr;
    _status.store(kIdle);
}

void* Dispatcher::threadEntry(void* args) {
    std::unique_ptr<Args> info(static_cast<Args*>(args));
    info->d->forever(info->index);
    return nullptr;
}

void Dispatcher::forever(size_t id) {
    Assert(id);
    ThreadInfo& info = _threadPool[id];
    while (true) {
        pthread_mutex_lock(&info.lock);
        pthread_mutex_unlock(&info.lock);
        info.ack.store(1);
        while (true) {
            Status s = _status.load();
            if (s == kRunning) {
                runQ(id);
                info.ack.store(0);
                break;
            } else if (s == kStop) {
                return;
            }
        }
    }
}

void Dispatcher::runQ(size_t id) {
    _threadPool[id].wrapper->runQ(_threadPool[id].q);
}

}
