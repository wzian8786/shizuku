#pragma once
#include "netlist_def.h"
#include "szk_object.h"
namespace netlist {
class Base : public util::Object<32, false> {
 public:
    typedef util::Object<32, false> Object;
    void setID(uint32_t id) { this->Object::operator=(Object(id)); }
    void getID() const { this->Object::operator*(); }

    constexpr static size_t kNextOffset = sizeof(util::Object<32, false>);
    static_assert(kNextOffset == 4, "size unmatch");
};

}
