#pragma once
#include <stdlib.h>
#include <functional>
#include "szk_dispatch.h"
namespace util {
template<typename T>
class TransBuilder {
 public:
    typedef T OT;
    OT& operator() (T& p) const { return p; } 
};

template<typename T>
class NoFilter {
 public:
    bool operator() (const T&) const { return false; }
};

template<typename T>
class ValidFilter {
 public:
    bool operator() (const T& t) const { return !t; }
};

template<typename Pool,
         typename Builder,
         typename Filter>
class ForeachFunc {
 public:
    typedef typename Pool::Element Element;
    typedef typename Builder::OT OT;
    typedef std::function<void(OT&, uint64_t)> Func;

    ForeachFunc(Func func, const Builder& builder=Builder(),
                           const Filter& filter=Filter()) :
                    _func(func), _builder(builder), _filter(filter) {}

    ForeachFunc(const ForeachFunc& another) :
                    _func(another._func),
                    _builder(another._builder),
                    _filter(another._filter) {}

    void operator() (uint32_t id) {
        OT& o = _builder(Pool::get()[id]);
        if (!_filter(o)) {
            _func(o, id);
        }
    }

 private:
    Func                            _func;
    Builder                         _builder;
    const Filter&                   _filter;
};

template<typename Pool,
         typename Builder=TransBuilder<typename Pool::Element>,
         typename Filter=NoFilter<typename Pool::Element>>
void foreach(std::function<void(typename Builder::OT&, uint64_t)> func,
             const Dispatcher::Range& range, size_t threads=0) {
    ForeachFunc<Pool, Builder, Filter> ffunc(func);
    Dispatcher::parallel_for(ffunc, range, threads);
}

template<typename Pool,
         typename Builder=TransBuilder<typename Pool::Element>,
         typename Filter=NoFilter<typename Pool::Element>>
void foreach(std::function<void(typename Builder::OT&, uint64_t)> func,
             size_t threads=0) {
    Dispatcher::Range range(0, Pool::getMaxSize());
    ForeachFunc<Pool, Builder, Filter> ffunc(func);
    Dispatcher::parallel_for(ffunc, range, threads);
}

template<typename Pool,
         typename Func,
         typename Builder,
         typename Filter>
class ReduceFunc {
 public:
    typedef typename Pool::Element Element;
    typedef typename Builder::OT OT;

    ReduceFunc(Func* func, const Builder& builder=Builder(),
                           const Filter& filter=Filter()) :
                    _func(func), _builder(builder),
                    _filter(filter), _delegate(true) {}

    ReduceFunc(const ReduceFunc& another) :
                    _func(new Func(*another._func, 0)),
                    _builder(another._builder),
                    _filter(another._filter),
                    _delegate(false) {}

    ~ReduceFunc() {
        if (!_delegate) delete _func;
    }

    void operator() (uint32_t id) {
        OT& o = _builder(Pool::get()[id]);
        if (!_filter(o)) {
            (*_func)(o, id);
        }
    }

    void join(ReduceFunc* another) {
        _func->join(*another->_func);
    }

 private:
    Func*                           _func;
    Builder                         _builder;
    const Filter&                   _filter;
    bool                            _delegate;
};

template<typename Pool,
         typename Func,
         typename Builder=TransBuilder<typename Pool::Element>,
         typename Filter=NoFilter<typename Pool::Element>>
void reduce(Func* func,
            const Dispatcher::Range& range, size_t threads=0) {
    ReduceFunc<Pool, Func, Builder, Filter> ffunc(func);
    Dispatcher::parallel_reduce(ffunc, range, threads);
}

template<typename Pool,
         typename Func,
         typename Builder=TransBuilder<typename Pool::Element>,
         typename Filter=NoFilter<typename Pool::Element>>
void reduce(Func* func, size_t threads=0) {
    Dispatcher::Range range(0, Pool::getMaxSize());
    ReduceFunc<Pool, Func, Builder, Filter> ffunc(func);
    Dispatcher::parallel_reduce(ffunc, range, threads);
}
}
