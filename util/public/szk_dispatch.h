#pragma once
#include <pthread.h>
#include <vector>
#include <cstdint>
#include <memory>
#include "szk_assert.h"
#include "szk_wsq.h"
namespace util {
class Dispatcher {
 public:
    static constexpr size_t kDefaultThreadNum = 8;
    explicit Dispatcher(size_t threadNum=kDefaultThreadNum);
    ~Dispatcher();

    static Dispatcher& ref() { return gDispatcher; }
    
    typedef std::pair<uint64_t, uint64_t> Range;
    
    class Wrapper {
     public:
        virtual ~Wrapper() {}
        virtual void runQ(Wsqueue& q) = 0;
        virtual void join(Wrapper* another) = 0;
    };

    template<typename Func>
    class ForeachWrapper : public Wrapper {
     public:
        ForeachWrapper(Func func, std::atomic<size_t>& tasks) :
                            _func(func),
                            _tasks(tasks) {}

        void runQ(Wsqueue& q) override {
            uint32_t t = q.pop();
            while (t) {
                _func(t - 1);
                Assert(_tasks.load());
                _tasks--;
                t = q.pop();
                while (!t) {
                    if (_tasks > Wsqueue::kReservedTasks) {
                        if (q.steal()) {
                            t = q.pop();
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        void join(Wrapper* another) override {}

     private:
        Func                        _func;
        std::atomic<size_t>&        _tasks;
    };

    template<typename Func>
    class ReduceWrapper : public Wrapper {
     public:
        ReduceWrapper(Func* func, std::atomic<size_t>& tasks, bool delegate) :
                            _func(func),
                            _tasks(tasks),
                            _delegate(delegate) {}

        ~ReduceWrapper() {
            if (!_delegate) delete _func;
        }

        void runQ(Wsqueue& q) override {
            uint32_t t = q.pop();
            while (t) {
                (*_func)(t - 1);
                Assert(_tasks.load());
                _tasks--;
                t = q.pop();
                while (!t) {
                    if (_tasks > Wsqueue::kReservedTasks) {
                        if (q.steal()) {
                            t = q.pop();
                            Assert(t);
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        void join(Wrapper* another) override {
            ReduceWrapper* a = static_cast<ReduceWrapper*>(another);
            _func->join(a->_func);
        }

     private:
        Func*                   _func;
        std::atomic<size_t>&    _tasks;
        bool                    _delegate;
    };

    template<typename Func>
    static void parallel_for(Func func, const Range& range, size_t threads=0) {
        Dispatcher& d = ref();
        threads = d.determinThreadNum(threads, range);
        d.init(threads);
        d.wrapperForeach<Func>(threads, func);
        d.run(threads, range);
    }

    template<typename Func>
    static void parallel_reduce(Func& func, const Range& range, size_t threads=0) {
        Dispatcher& d = ref();
        threads = d.determinThreadNum(threads, range);
        d.init(threads);
        d.wrapperReduce<Func>(threads, &func);
        d.run(threads, range);
    }

 private:
    typedef enum {
        kIdle = 0,
        kRunning,
        kStop
    } Status;

    struct Args {
        Dispatcher* d;
        size_t      index;
    };

    struct ThreadInfo {
        ThreadInfo() : pid(),
                       lock(PTHREAD_MUTEX_INITIALIZER),
                       ack(0),
                       wrapper(nullptr) {}
        ThreadInfo(ThreadInfo&& info) :
                       pid(info.pid),
                       lock(info.lock),
                       ack(info.ack.load()),
                       q(info.q),
                       wrapper(std::move(info.wrapper)) {}

        ~ThreadInfo() {
            pthread_mutex_destroy(&lock);
        }
        pthread_t                   pid;
        pthread_mutex_t             lock;
        std::atomic<uint8_t>        ack;
        Wsqueue                     q;
        std::unique_ptr<Wrapper>    wrapper;
    };

    template<typename Func>
    void wrapperForeach(size_t threads, Func func) {
        for (size_t i = 0; i < threads; ++i) {
            _threadPool[i].wrapper = std::unique_ptr<Wrapper>(new ForeachWrapper<Func>(func, _tasks));
        }
    }

    template<typename Func>
    void wrapperReduce(size_t threads, Func* func) {
        for (size_t i = 0; i < threads; ++i) {
            func = i ? new Func(*func) : func;
            _threadPool[i].wrapper = std::unique_ptr<Wrapper>(
                                        new ReduceWrapper<Func>(func, _tasks, i==0));
        }
    }

    void init(size_t threads);
    void run(size_t threads, const Range& range);
    void forever(size_t id);
    void runQ(size_t);

    void lock(size_t end=0, size_t begin=1);
    void unlock(size_t end=0, size_t begin=1);
    size_t determinThreadNum(size_t threads, const Range& range);

    static void* threadEntry(void* args);

 private:
    std::atomic<Status>                 _status;
    std::atomic<size_t>                 _tasks;
    std::vector<ThreadInfo>             _threadPool;

    static Dispatcher                   gDispatcher;
    static const char*                  kThreadNumEnv;
};
}

