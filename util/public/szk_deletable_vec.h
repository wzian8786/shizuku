#pragma once
namespace util {
template <typename T>
class DeletableVec {
 public:
    DeletableVec() : _deleted(0) {}
    explicit DeletableVec(size_t n) : _vec(n), _deleted(n) {}

    template<class... Args>
    void emplace_back(Args&&... args) {
        T t(std::forward<Args>(args)...);
        if (!t) _deleted++;
        _vec.push_back(std::move(t));
    }

    template<class... Args>
    void assign(size_t id, Args&&... args) {
        if (!_vec[id]) _deleted--;
        T t(std::forward<Args>(args)...);
        if (!t) _deleted++;
        _vec[id] = std::move(t);
    }

    T del(size_t id) {
        T ret = 0;
        if (_vec[id]) {
            ret = std::move(_vec[id]);
            _vec[id] = 0;
            _deleted++;
        }
        return ret;
    }

    const T& operator[](size_t id) const { return _vec[id]; }

    size_t size() const { return _vec.size(); }
    size_t validSize() const { return _vec.size() - _deleted; }
    bool empty() const { return _vec.empty(); }

    template<bool C>
    struct IT {
        typedef typename std::conditional<C, const T, T>::type rtype;
        typedef typename std::conditional<C, const std::vector<T>,
                                        std::vector<T>>::type vectype;

        IT(vectype* v, size_t i) : index(i), vec(v) {
            while (index < vec->size()) {
                if ((*vec)[index]) break;
                index++;
            }
        }

        rtype& operator ->() const {
            return (*vec)[index];
        }

        bool operator ==(const IT& a) const {
            return vec == a.vec && index == a.index;
        }

        bool operator !=(const IT& a) const {
            return vec != a.vec || index != a.index;
        }

        rtype& operator *() const {
            return (*vec)[index];
        }

        IT& operator ++() {
            while (index < vec->size()) {
                index++;
                if ((*vec)[index]) break;
            }
            return *this;
        }

        IT operator ++(int) {
            IT ret = *this;
            while (index < vec->size()) {
                index++;
                if ((*vec)[index]) break;
            }
            return ret;
        }

        size_t                  index;
        vectype*                vec;
    };

    typedef IT<false> iterator;
    typedef IT<true> const_iterator;

    iterator begin() { return iterator(&_vec, 0); }
    iterator end() { return iterator(&_vec, _vec.size()); }

    const_iterator begin() const { return const_iterator(&_vec, 0); }
    const_iterator end() const { return const_iterator(&_vec, _vec.size()); }

 private:
    std::vector<T>              _vec;
    size_t                      _deleted;
};
}
