#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include "szk_pool.h"
#include "szk_parray.h"
#include "vid.h"
namespace vid {
typedef enum {
    VT_SHORT = 0,
    VT_LONG,
    VT_ESCAPED_SHORT,
    VT_ESCAPED_LONG,
    VT_DERIVED,
} VidType;

template<uint32_t Namespace>
class VidDB {
 public:
    VidDB();

 private:
    struct Spec {
        static constexpr uint32_t kLocalBits  = 10;
        static constexpr uint32_t kBlockBits  = 8;
        static constexpr uint32_t kPageIDBits = 12;
        typedef util::PArray<char**, kPageIDBits> PagePtrVec;
    };
    typedef util::Pool<char*, Namespace, Spec> LongIdPool;

 public:
    constexpr static size_t kShortVidCharNum = 7;

    static VidDB& get() { return gSingleton; }
    static const VidDB& getConst() { return gSingleton; }

    typename Vid<Namespace>::Base getVid(const char* s, bool force);
    typename Vid<Namespace>::Base getVid(const std::string& s, bool force) {
        return getVid(s.c_str(), force);
    }
    const char* getLongStr(size_t index) const {
        return LongIdPool::get()[index];
    }

    void predef(const std::vector<std::pair<int, std::string> >& pd);
    Vid<Namespace> getPredefVid(int pid) const;

 private:
    typedef std::pair<std::string, bool> NormalizeInfo;
    NormalizeInfo normalize(const char* s, bool force) const;
    inline void normalizeUnescaped(const char* s, NormalizeInfo& info, bool force) const;
    inline void normalizeEscaped(const char* s, NormalizeInfo& info) const;

 private:

    typedef std::unordered_map<std::string, uint64_t> StrMap;
    typedef util::PArray<std::unique_ptr<const char>, 24> StrVec;
    typedef util::PArray<Vid<Namespace>, 24> VidVec;

    static VidDB                    gSingleton;
    StrMap                          _longIdMap;

    VidVec                          _derivedIdVec;
    Vid<Namespace>                  _nextDerivedId;

    std::vector<Vid<Namespace>>     _predef;

    std::mutex                      _lock;
};

template<uint32_t Namespace>
VidDB<Namespace> VidDB<Namespace>::gSingleton;
}
