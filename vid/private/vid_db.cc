#include "vid_db.h"
#include <string.h>
#include <sstream>
#include "szk_assert.h"
namespace vid {

template<uint32_t Namespace>
VidDB<Namespace>::VidDB() : _nextDerivedId(VT_DERIVED, 1) {}

static inline bool isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n';
}

static inline bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

static inline bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isEscaped(const char* s) {
    size_t len = strlen(s);
    if (len >= 2) {
        bool esp = (s[0] == '\\' && isSpace(s[len-1]));
        Assert(!esp || !isSpace(s[len-2]));
        return esp;
    }
    return false;
}

bool needEscape(const char* s, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (!((isAlpha(c) || c == '_') ||
              (i && (isDigit(c) || c == '$')))) {
            return true;
        }
    }
    return false;
}

template<uint32_t Namespace>
typename VidDB<Namespace>::NormalizeInfo VidDB<Namespace>::normalize(const char* s, bool force) const {
    NormalizeInfo info{"", false};
    if (isEscaped(s)) {
        normalizeEscaped(s, info);
    } else {
        normalizeUnescaped(s, info, force);
    }
    return info;
}

template<uint32_t Namespace>
void VidDB<Namespace>::normalizeEscaped(const char* s, NormalizeInfo& info) const {
    std::stringstream ss;
    size_t len = strlen(s);
    Assert(len > 2);

    if (needEscape(s+1, len-2)) {
        for (size_t i = 0; i < len-2; ++i) {
            if (isSpace(s[i+1])) {
                ss << "\\S";
            } else {
                ss << s[i+1];
            }
        }
        info.first = ss.str();
        info.second = true;
    } else {
        info.first = std::string(s+1, len-2);
    }
}

template<uint32_t Namespace>
void VidDB<Namespace>::normalizeUnescaped(const char* s, NormalizeInfo& info, bool force) const {
    std::stringstream ss;
    size_t len = strlen(s);
    Assert(len);

    if (needEscape(s, len)) {
        Assert(force);
        for (size_t i = 0; i < len; ++i) {
            if (isSpace(s[i])) {
                ss << "\\S";
            } else {
                ss << s[i];
            }
        }
        info.first = ss.str();
        info.second = true;
    } else {
        info.first = std::string(s);
    }
}

template<uint32_t Namespace>
typename Vid<Namespace>::Base VidDB<Namespace>::getVid(const char* s, bool force) {
    NormalizeInfo info = normalize(s, force);
    const std::string& str = info.first;
    bool escaped = info.second;
    size_t len = str.length();
    if (len <= kShortVidCharNum) {
        uint64_t addr = 0;
        strncpy((char*)&addr, str.c_str(), len);
        if (!escaped) {
            return typename Vid<Namespace>::Base(VT_SHORT, addr);
        } else {
            return typename Vid<Namespace>::Base(VT_ESCAPED_SHORT, addr);
        }
    } else {
        std::lock_guard<std::mutex> lg(_lock);
        (void) lg;
        auto it = _longIdMap.find(str);
        uint64_t addr = 0;
        if (it != _longIdMap.end()) {
            addr = it->second;
        } else {
            addr = LongIdPool::get().New();
            LongIdPool::get()[addr] = strdup(str.c_str());
            _longIdMap.emplace(str, addr);
        }
        if (!escaped) {
            return typename Vid<Namespace>::Base(VT_LONG, addr);
        } else {
            return typename Vid<Namespace>::Base(VT_ESCAPED_LONG, addr);
        }
    }
}

template<uint32_t Namespace>
void VidDB<Namespace>::predef(const std::vector<std::pair<int, std::string> >& pd) {
    for (int i = 0; i < (int)pd.size(); ++i) {
        const std::pair<int, std::string>& p = pd[i];
        Assert(i == p.first);
        _predef.emplace_back(getVid(p.second.c_str(), false));
    }
}

template<uint32_t Namespace>
Vid<Namespace> VidDB<Namespace>::getPredefVid(int pid) const {
    size_t sid = (size_t) pid;
    Assert(sid < _predef.size());
    return _predef[sid];
}

template class VidDB<VN_DEFAULT>;
}
