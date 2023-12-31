#define BOOST_TEST_MODULE NETLIST_READER_TEST
#include <stdio.h>
#include <sstream>
#include <boost/test/unit_test.hpp>
#include "nl_reader.h"
#include "nl_datatype.h"
#include "nl_datatype_db.h"
#include "nl_netlist.h"
#include "nl_folded_obj.h"
#include "nl_flat_obj.h"
#include "szk_foreach.h"
using netlist::Scalar;
using netlist::Real;
using netlist::DataType;
using netlist::PackedArray;
using netlist::DataTypeDB;
using Port = netlist::Port<0>;
using Net = netlist::Net<0>;
using Module = netlist::Module<0>;
using Netlist = netlist::Netlist<0>;
using FMInst = netlist::FMInst<0>;
using FPInst = netlist::FPInst<0>;
using FDPort = netlist::FDPort<0>;
using FRPort = netlist::FRPort<0>;
using netlist::Vid;
std::vector<std::pair<Vid, Port::Direction>> eport = {
    { "p1", Port::kPortInput },
    { "p2", Port::kPortOutput },
    { "p3", Port::kPortInout },
    { "p4", Port::kPortInput },
    { "p5", Port::kPortInput },
    { "w", Port::kPortInput },
    { "w", Port::kPortOutput },
};
std::vector<Vid> enet = { "n1", "n2", "n3" };
std::vector<Vid> emodule = { "S$Root", "m1", "m2", "m3", "m4"};
std::vector<std::string> epath = {
    "",
    "m1",
    "m1.i1",
    "m1.i2",
    "m1.i3",
};

std::vector<std::string> epath1 = {
    "m1.i4",
    "m1.i5",
    "m1.i6",
};

std::vector<std::string> epath2 = {
    "m1.i4.i0",
    "m1.i5.i0",
    "m1.i6.i0",
};

BOOST_AUTO_TEST_CASE ( test_netlist_reader ) {
    std::stringstream ss;
    ss << TEST_SOURCE << "/test.nl";
    FILE* fp = fopen(ss.str().c_str(), "r");
    netlist::reader::NetlistReader::parse(fp);
    fclose(fp);
    netlist::reader::NetlistReader::finalize();

    Port::foreach([](const Port& port, size_t i) {
        if (i >= eport.size()) {
            BOOST_CHECK(0);
        } else {
            if (i < 5) {
                BOOST_CHECK(port.getName() == eport[i].first);
            }
            BOOST_CHECK(port.getDirection() == eport[i].second);
        }
    }, 1);

    Net::foreach([](const Net& net, size_t i) {
        if (i >= enet.size()) {
            BOOST_CHECK(0);
        } else {
            BOOST_CHECK(net.getName() == enet[i]);
        }
    }, 1);

    Module::foreach([](const Module& module, size_t i) {
        if (i >= emodule.size()) {
            BOOST_CHECK(0);
        } else {
            BOOST_CHECK(module.getName() == emodule[i]);
        }
    }, 1);

    Netlist& netlist = Netlist::get();
    netlist.elab();

    FMInst::foreach([](FMInst& inst, uint64_t id){
        if (id >= epath.size()) {
            BOOST_CHECK(0);
        } else {
            BOOST_CHECK(inst.getPath() == epath[id]);
        }
    }, 1);

    size_t index = 0;
    FPInst::foreach([&index](FPInst& inst, uint64_t id) {
        BOOST_CHECK(epath1[index++] == inst.getPath());
    }, 1);

    index = 0;
    FRPort::foreach([&index](FRPort port, uint64_t id) {
        BOOST_CHECK(epath2[index++] == port.getPath());
        if (port.getDriver()) {
            BOOST_CHECK(port.getDriver().getPath() == "m1.i4.o0");
        }
    }, 1);
}

BOOST_AUTO_TEST_CASE ( test_datatype ) {
    DataTypeDB dtdb;

    Scalar s1;
    const DataType& s2s = dtdb.persist(&s1);
    BOOST_CHECK(s2s == dtdb.getTypeScalar());

    DataType::Equal equal;
    Scalar s2;
    Real s3;

    BOOST_CHECK(equal(&s1, &s2));
    BOOST_CHECK(!equal(&s1, &s3));
    BOOST_CHECK(equal(&s3, &dtdb.getTypeReal()));

    PackedArray p1(-3, 0, false);
    PackedArray p2(-3, 0, false);
    PackedArray p3(-4, 0, false);
    PackedArray p4(-3, 1, false);
    PackedArray p5(-3, 0, true);
    BOOST_CHECK(equal(&p1, &p2));
    BOOST_CHECK(!equal(&p1, &p3));
    BOOST_CHECK(!equal(&p1, &p4));
    BOOST_CHECK(!equal(&p1, &p5));
    BOOST_CHECK(dtdb.persist(&p1) == dtdb.persist(&p1));
    BOOST_CHECK(dtdb.persist(&p1) == dtdb.persist(&p2));
    BOOST_CHECK(dtdb.persist(&p1) != dtdb.persist(&p3));
}
