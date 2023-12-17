#pragma once
#include "szk_object.h"
#include <string>
#include <vector>
namespace vid {
typedef enum {
    VN_DEFAULT = 0,
} VidNamespace;

template<uint32_t Namespace>
class Vid : public util::Object<56, true> {
 public:
    struct Hash {
        size_t operator ()(Vid vid) const {
            return std::hash<size_t>()(*vid);
        } 
    };

    typedef util::Object<56, true> Base;
    Vid() {}
    Vid(const char* s, bool force=false);
    Vid(const std::string& s, bool force=false);
    Vid(uint64_t rtti, uint64_t id) : Base(rtti, id) {}
    explicit Vid(int pid);
    explicit Vid(Base id) : Base(id) {}

    Vid derive() const;
    std::string str() const;

    uint64_t operator *() const { return this->Base::operator*(); }
    bool operator ==(Vid rhs) const { return this->Base::operator==(rhs); }
    bool operator !=(Vid rhs) const { return this->Base::operator!=(rhs); }
    bool valid() const { return this->Base::valid(); }
    bool operator !() const { return this->Base::operator!(); }
    bool operator <(Vid rhs) const { return this->Base::operator<(rhs); }

    static void predef(const std::vector<std::pair<int, std::string> >& pd);
};

typedef Vid<VN_DEFAULT> VID;
}
