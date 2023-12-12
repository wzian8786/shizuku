#define BOOST_TEST_MODULE NETLIST_READER_TEST
#include <stdio.h>
#include <sstream>
#include <boost/test/unit_test.hpp>
#include "netlist_reader.h"
#include "nl_datatype.h"
#include "nl_datatype_db.h"
#include "szk_foreach.h"
#include "nl_netlist.h"
#include "nl_folded_obj.h"
using netlist::Scalar;
using netlist::Real;
using netlist::DataType;
using netlist::PackedArray;
using netlist::DataTypeDB;
using Port = netlist::Port<0>;
using Net = netlist::Net<0>;
using Module = netlist::Module<0>;
BOOST_AUTO_TEST_CASE ( test_netlist_reader ) {
    std::stringstream ss;
    ss << TEST_SOURCE << "/test.nl";
    FILE* fp = fopen(ss.str().c_str(), "r");
    netlist::reader::NetlistReader::parse(fp);
    fclose(fp);

    Port::foreach([](const Port& port, size_t i) {
        if (i == 0) {
            BOOST_CHECK(port.getName() == "p1");
            BOOST_CHECK(port.getDirection() == Port::kPortInput);
        } else if (i == 1) {
            BOOST_CHECK(port.getName() == "p2");
            BOOST_CHECK(port.getDirection() == Port::kPortOutput);
        } else if (i == 2) {
            BOOST_CHECK(port.getName() == "p3");
            BOOST_CHECK(port.getDirection() == Port::kPortInout);
        }
    }, 1);

    Net::foreach([](const Net& net, size_t i) {
        BOOST_CHECK(net.getName() == "n1");
    }, 1);

    Module::foreach([](const Module& module, size_t i) {
        if (i == 0) {
            BOOST_CHECK(module.getName() == "c");
        } else if (i == 1) {
            BOOST_CHECK(module.getName() == "e");
        } else if (i == 2) {
            BOOST_CHECK(module.getName() == "a");
        }
    }, 1);
}

BOOST_AUTO_TEST_CASE ( test_datatype ) {
    DataTypeDB dtdb;

    Scalar s1;
    const DataType* s2s = dtdb.persist(&s1);
    BOOST_CHECK(s2s == dtdb.getTypeScalar());

    DataType::Equal equal;
    Scalar s2;
    Real s3;

    BOOST_CHECK(equal(&s1, &s2));
    BOOST_CHECK(!equal(&s1, &s3));
    BOOST_CHECK(equal(&s3, dtdb.getTypeReal()));

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
