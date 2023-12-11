#define BOOST_TEST_MODULE NETLIST_READER_TEST
#include <stdio.h>
#include <sstream>
#include <boost/test/unit_test.hpp>
#include "netlist_reader.h"
#include "nl_datatype.h"
#include "nl_datatype_db.h"
using netlist::Scalar2S;
using netlist::Scalar4S;
using netlist::DataType;
using netlist::PackedArray;
using dtdb = netlist::DataTypeDB;
BOOST_AUTO_TEST_CASE ( test_netlist_reader ) {
    std::stringstream ss;
    ss << TEST_SOURCE << "/test.nl";
    FILE* fp = fopen(ss.str().c_str(), "r");
    netlist::reader::NetlistReader::parse(fp);
    fclose(fp);
}

BOOST_AUTO_TEST_CASE ( test_datatype ) {
    Scalar2S s1;
    const DataType* s2s = dtdb::get().persist(&s1);
    BOOST_CHECK(s2s == dtdb::get().getScalar2S());

    DataType::Equal equal;
    Scalar2S s2;
    Scalar4S s3;

    BOOST_CHECK(equal(&s1, &s2));
    BOOST_CHECK(!equal(&s1, &s3));
    BOOST_CHECK(equal(&s3, dtdb::get().getScalar4S()));

    PackedArray p1(-3, 0, false);
    PackedArray p2(-3, 0, false);
    PackedArray p3(-4, 0, false);
    PackedArray p4(-3, 1, false);
    PackedArray p5(-3, 0, true);
    BOOST_CHECK(equal(&p1, &p2));
    BOOST_CHECK(!equal(&p1, &p3));
    BOOST_CHECK(!equal(&p1, &p4));
    BOOST_CHECK(!equal(&p1, &p5));
    BOOST_CHECK(dtdb::get().persist(&p1) == dtdb::get().persist(&p1));
    BOOST_CHECK(dtdb::get().persist(&p1) == dtdb::get().persist(&p2));
    BOOST_CHECK(dtdb::get().persist(&p1) != dtdb::get().persist(&p3));
}
