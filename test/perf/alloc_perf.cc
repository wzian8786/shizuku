#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstdlib>
#include "szk_pool.h"
struct Spec {
    static constexpr uint32_t kLocalBits = 10;
    static constexpr uint32_t kBlockBits = 8;
    static constexpr uint32_t kPageIDBits = 21;
};
typedef util::Pool<size_t, 0, Spec> Pool;
#define kAllocNum 10000000
void* allocateMemUsePool(void* args) {
    std::vector<size_t*>* vec = (std::vector<size_t*>*) args;
    for (size_t i = 0; i < kAllocNum; ++i) {
        uint64_t addr = Pool::get().New();
        (*vec)[i] = &Pool::get()[addr];
    }
    return nullptr;
}

void* allocateMemUseMalloc(void* args) {
    std::vector<size_t*>* vec = (std::vector<size_t*>*) args;
    for (size_t i = 0; i < kAllocNum; ++i) {
        (*vec)[i] = (size_t*)malloc(sizeof(size_t));
    }
    return nullptr;
}

int main() {
    constexpr size_t threads = 8;
    std::vector<pthread_t> pids(threads);
    timeval begin, end;
    std::vector<size_t*> address(kAllocNum);
    gettimeofday(&begin, nullptr);
    for (size_t i = 0; i < threads; ++i) {
        pthread_create(&pids[i], nullptr, allocateMemUsePool, &address);
    }
    for (size_t i = 0; i < threads; ++i) {
        pthread_join(pids[i], nullptr);
    }
    gettimeofday(&end, nullptr);
    double t1 = (((double)end.tv_sec) * 1000000 + end.tv_usec -
                 ((double)begin.tv_sec) * 1000000 - begin.tv_usec) / 1000000;
    printf("Pool took %.2f seconds\n", t1);

    printf("pid %d\n", getpid());
    sleep(10);
                
    gettimeofday(&begin, nullptr);
    for (size_t i = 0; i < threads; ++i) {
        pthread_create(&pids[i], nullptr, allocateMemUseMalloc, &address);
    }
    for (size_t i = 0; i < threads; ++i) {
        pthread_join(pids[i], nullptr);
    }
    gettimeofday(&end, nullptr);
    double t2 = (((double)end.tv_sec) * 1000000 + end.tv_usec -
                 ((double)begin.tv_sec) * 1000000 - begin.tv_usec) / 1000000;
    printf("malloc took %.2f seconds\n", t2);
    return 0;
}
