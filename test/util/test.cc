#define BOOST_TEST_MODULE INFRA_TEST
#include <stdlib.h>
#include <time.h>
#include <boost/test/unit_test.hpp>
#include "szk_object.h"
#include "szk_pool.h"
#include "szk_parray.h"
#include "szk_foreach.h"
#include "szk_wsq.h"
static size_t getThreadNum() {
    size_t threads = 8;
    const char* env = getenv("SHIZUKU_THREAD_NUM");
    if (env && (*env)) {
        threads = strtoul(env, nullptr, 0);
    }
    return threads ? threads : 8;
}

BOOST_AUTO_TEST_CASE ( test_jobject ) {
    util::Object<8, false> obja1(42);
    BOOST_CHECK(sizeof(obja1) == 4);
    BOOST_CHECK(*obja1 == 42);
    BOOST_CHECK(obja1.getAddr() == 42);

    util::Object<8, true> obja2(1, 42);
    BOOST_CHECK(sizeof(obja2) == 4);
    BOOST_CHECK(*obja2 == 0x100002a);
    BOOST_CHECK(obja2.getAddr() == 42);
    BOOST_CHECK(obja2.getRtti() == 1);

    util::Object<24, false> objb1(42);
    BOOST_CHECK(sizeof(objb1) == 4);
    BOOST_CHECK(*objb1 == 42);
    BOOST_CHECK(objb1.getAddr() == 42);

    util::Object<24, true> objb2(1, 42);
    BOOST_CHECK(sizeof(objb2) == 4);
    BOOST_CHECK(*objb2 == 0x100002a);
    BOOST_CHECK(objb2.getAddr() == 42);
    BOOST_CHECK(objb2.getRtti() == 1);

    util::Object<32, false> objc1(42);
    BOOST_CHECK(sizeof(objc1) == 4);
    BOOST_CHECK(*objc1 == 42);
    BOOST_CHECK(objc1.getAddr() == 42);

    util::Object<32, true> objc2(1, 42);
    BOOST_CHECK(sizeof(objc2) == 8);
    BOOST_CHECK(*objc2 == 0x10000000000002a);
    BOOST_CHECK(objc2.getAddr() == 42);
    BOOST_CHECK(objc2.getRtti() == 1);

    util::Object<40, false> objd1(42);
    BOOST_CHECK(sizeof(objd1) == 8);
    BOOST_CHECK(*objd1 == 42);
    BOOST_CHECK(objd1.getAddr() == 42);

    util::Object<40, true> objd2(1, 42);
    BOOST_CHECK(sizeof(objd2) == 8);
    BOOST_CHECK(*objd2 == 0x10000000000002a);
    BOOST_CHECK(objd2.getAddr() == 42);
    BOOST_CHECK(objd2.getRtti() == 1);

    util::Object<56, false> obje1(42);
    BOOST_CHECK(sizeof(obje1) == 8);
    BOOST_CHECK(*obje1 == 42);
    BOOST_CHECK(obje1.getAddr() == 42);

    util::Object<56, true> obje2(1, 42);
    BOOST_CHECK(sizeof(obje2) == 8);
    BOOST_CHECK(*obje2 == 0x10000000000002a);
    BOOST_CHECK(obje2.getAddr() == 42);
    BOOST_CHECK(obje2.getRtti() == 1);
}

struct Spec {
    static constexpr uint32_t kLocalBits = 10;
    static constexpr uint32_t kBlockBits = 4;
    static constexpr uint32_t kPageIDBits = 21;
};

typedef util::Pool<int, 0, Spec> Pool;

template<typename P>
class JPoolTester {
 public:
    explicit JPoolTester(P& pool) : _pool(pool), _hasError(false) {}
    P& getPool() const { return _pool; }
    typename P::PagePtr getPage(uint64_t index) const {
        assert(index < _pool._pt.size());
        return _pool._pt[index];
    }
    uint64_t getMapped() const { return _pool._mapped; }
    void setHasError() { _hasError = true; }
    bool hasError() const { return _hasError; }
    uint64_t newBlock(size_t blocks) const { return _pool.newBlock(blocks); }

 private:
    P&              _pool;
    bool            _hasError;
};

static void* jpoolRoutine(void* args) {
    JPoolTester<Pool>* tester = static_cast<JPoolTester<Pool>*>(args);
    Pool& pool = tester->getPool();
    for (size_t i = 0; i < 10000; ++i) {
        uint64_t addr = tester->newBlock((rand() % 15) + 1);
        uint64_t base = addr >> Pool::kPageBits;
        pool[addr] = i;
        if (pool[addr] != (int)i) {
            tester->setHasError();
        }
        if (base > tester->getMapped()) {
            tester->setHasError();
        }
    }
    return nullptr;
}

BOOST_AUTO_TEST_CASE ( test_jpool ) {
    srand(time(nullptr));
    Pool& pool = Pool::get();
    pool.clear();
    size_t threads = getThreadNum();
    std::vector<pthread_t> pids(threads);

    JPoolTester<Pool> tester(pool);

    for (size_t i = 0; i < threads; ++i) {
        pthread_create(&pids[i], nullptr, jpoolRoutine, &tester);
    }
    
    for (size_t i = 0; i < threads; ++i) {
        pthread_join(pids[i], nullptr);
    }

    uint64_t addr = tester.newBlock(0);
    if (!addr) return;

    uint64_t pages = ((addr - 1) >> Pool::kPageBits) + 1;
    for (uint64_t i = 0; i < pages; ++i) {
        BOOST_CHECK(tester.getPage(i));
    }
    BOOST_CHECK(tester.getPage(pages) == nullptr);
    BOOST_CHECK(!tester.hasError());
}

BOOST_AUTO_TEST_CASE ( test_jparray ) {
    util::PArray<int, 20> array;
    array.resize(100);
    array.resize(0);
    for (int i = 0; i < 10000; ++i) {
        array.push_back(i);
        BOOST_CHECK(i == array[i]);
        BOOST_CHECK((i+1) == (int)array.size());
    }
}

BOOST_AUTO_TEST_CASE ( test_alloc ) {
    Pool& pool = Pool::get();
    pool.clear();
    uint64_t addr = 0;
    for (size_t i = 0; i < 10000; i++) {
        uint64_t size = (rand() % 100000) + 1;
        BOOST_CHECK(addr == pool.New(size));
        addr += size;
    }
}

class Sum {
 public:
    explicit Sum(const std::vector<size_t>& vec) :
                              _vec(vec),
                              _v(0) {}

    Sum(const Sum& another) : _vec(another._vec),
                              _v(0) {}

    void operator() (size_t id) {
        _v += _vec[id];
    }

    void join(Sum* another) {
        _v += another->_v;
    }
    size_t getTotal() const { return _v; }

 private:
    const std::vector<size_t>&  _vec;
    size_t                      _v;
};

class PSum {
 public:
    PSum() : _v(0) {}

    PSum(const PSum& another, unsigned) : _v(0) {}

    void operator() (const size_t& i, uint64_t index) {
        _v += i;
    }

    void join(PSum& another) {
        _v += another._v;
    }
    size_t getTotal() const { return _v; }

 private:
    size_t                      _v;
};

typedef util::Pool<size_t, 1, Spec> Pool1;
BOOST_AUTO_TEST_CASE ( test_foreach_reduce ) {
    std::vector<size_t> myvec(100);
    util::Dispatcher::Range range(0, myvec.size());
    util::Dispatcher::parallel_for([&myvec](size_t i) {
        myvec[i] = i;
    }, range);

    for (size_t i = 0; i < myvec.size(); ++i) {
        BOOST_CHECK(myvec[i] == i);
    }

    for (size_t i = 0; i < 1000; ++i) {
        Sum sum(myvec);
        util::Dispatcher::parallel_reduce(sum, range);
        BOOST_CHECK(sum.getTotal() == 4950);
    }

    Pool1::get().New(100);

    util::foreach<Pool1>([](size_t& v, size_t i) {
        if (i < 100) {
            v = i;
        }
    });

    for (size_t i = 0; i < 100; ++i) {
        BOOST_CHECK(Pool1::get()[i] == i);
    }

    PSum psum;
    util::reduce<Pool1>(&psum);
    BOOST_CHECK(psum.getTotal() == 100 * 99 / 2);
}

typedef struct {
    std::atomic<size_t>* tasks;
    std::vector<std::atomic<uint8_t> >* visited;
    util::Wsqueue* queue;
} WsqueueTestInfo;

static void* wsqueueRoutine(void* args) {
    WsqueueTestInfo* info = (WsqueueTestInfo*) args;
    while (*info->tasks) {
        uint32_t t = info->queue->pop();
        if (t) {
            uint8_t visited = (*info->visited)[t].load();
            BOOST_CHECK(!visited);
            (*info->visited)[t].store(1);
            (*info->tasks)--;
        } else {
            info->queue->steal();
        }
    }
    return nullptr;
}

BOOST_AUTO_TEST_CASE ( test_wsqueue ) {
    size_t threads = getThreadNum();
    size_t r = 1;
    std::vector<size_t> range = { r };
    for (size_t i = 0; i < threads; ++i) {
        r += (rand() % 1000) + 1;
        range.push_back(r); 
    }
    assert(range.size() == threads + 1);
    std::atomic<size_t> tasks(range.back() - 1);
    std::vector<std::atomic<uint8_t> > visited(range.back());

    std::vector<util::Wsqueue> queues(threads);
    std::vector<WsqueueTestInfo> info;
    for (size_t i = 0; i < threads; ++i) {
        queues[i].reset(range[i], range[i+1]);
        queues[i].addVictim(&queues[(i+1) % threads]);
        queues[i].addVictim(&queues[(i+2) % threads]);
        queues[i].addVictim(&queues[(i+3) % threads]);
        info.emplace_back(WsqueueTestInfo{&tasks, &visited, &queues[i]});
    }

    std::vector<pthread_t> pids(threads);
    for (size_t i = 0; i < threads; ++i) {
        pthread_create(&pids[i], nullptr, wsqueueRoutine, &info[i]);
    }
    
    for (size_t i = 0; i < threads; ++i) {
        pthread_join(pids[i], nullptr);
    }

    for (size_t i = 1; i < range.back(); ++i) {
        BOOST_CHECK(visited[i] == 1);
    }
}
