#include "vid.h"
#include <sstream>
#include "szk_assert.h"
#include "vid_db.h"
namespace vid {

template<uint32_t Namespace>
Vid<Namespace>::Vid(const char* s, bool force) :
            Base(VidDB<Namespace>::get().getVid(s, force)) {}

template<uint32_t Namespace>
Vid<Namespace>::Vid(const std::string& s, bool force) :
            Base(VidDB<Namespace>::get().getVid(s.c_str(), force)) {}

template<uint32_t Namespace>
Vid<Namespace>::Vid(int pid) :
            Base(VidDB<Namespace>::get().getPredefVid(pid)) {}

template<uint32_t Namespace>
std::string Vid<Namespace>::str() const {
    std::stringstream ss;
    switch (getRtti()) {
    case VT_SHORT: {
        uint64_t v = operator*();
        char* p = (char*)&v;
        char c;
        while ((c = *(p++))) {
            ss << c;
        }
        break;
    }

    case VT_ESCAPED_SHORT: {
        ss << '\\';
        uint64_t v = getAddr();
        char* p = (char*)&v;
        char c;
        while ((c = *(p++))) {
            ss << c;
        }
        ss << ' ';
        break;
    }

    case VT_LONG:
        return std::string(VidDB<Namespace>::getConst().getLongStr(getAddr()));

    case VT_ESCAPED_LONG:
        ss << '\\';
        ss << VidDB<Namespace>::getConst().getLongStr(getAddr());
        ss << ' ';
        break;

    default:
        Assert(0);
    }
    return ss.str();
}

template<uint32_t Namespace>
void Vid<Namespace>::predef(const std::vector<std::pair<int, std::string> >& pd) {
    VidDB<Namespace>::get().predef(pd);
}

template class Vid<VN_DEFAULT>;
}
