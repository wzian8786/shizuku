#define BOOST_TEST_MODULE VID_TEST
#include <boost/test/unit_test.hpp>
#include "vid.h"
typedef vid::Vid<vid::VN_DEFAULT> Vid;
BOOST_AUTO_TEST_CASE ( test_vid_normalize ) {
    BOOST_CHECK(Vid("\\aaa ").str() == "aaa");

    BOOST_CHECK(Vid("$fds", true).str() == "\\$fds ");

    BOOST_CHECK(Vid("\\*fds\t").str() == "\\*fds ");

    BOOST_CHECK(Vid("\\fds*fds\t").str() == "\\fds*fds ");

    BOOST_CHECK(Vid("fds fds", true).str() == "\\fds\\Sfds ");

    BOOST_CHECK(Vid("1234567$fds", true).str() == "\\1234567$fds ");

    BOOST_CHECK(Vid("\\1234567*fds\t").str() == "\\1234567*fds ");

    BOOST_CHECK(Vid("\\1234567fds*fds\t").str() == "\\1234567fds*fds ");

    BOOST_CHECK(Vid("1234567fds fds", true).str() == "\\1234567fds\\Sfds ");
}

BOOST_AUTO_TEST_CASE ( test_vid ) {
    Vid vid1("aaa");
    Vid vid2(std::string("aaa"));
    BOOST_CHECK(vid1 == vid2);

    BOOST_CHECK(vid1.str() == std::string("aaa"));

    Vid vid3("\\** ");
    Vid vid4(std::string("\\** "));
    BOOST_CHECK(vid3 == vid4);

    BOOST_CHECK(vid3.str() == std::string("\\** "));

    Vid vid5("aaa1234567");
    Vid vid6(std::string("aaa1234567"));
    BOOST_CHECK(vid5 == vid6);

    BOOST_CHECK(vid5.str() == std::string("aaa1234567"));

    Vid vid7("\\**1234567 ");
    Vid vid8(std::string("\\**1234567 "));
    BOOST_CHECK(vid7 == vid8);

    BOOST_CHECK(vid7.str() == std::string("\\**1234567 "));

    Vid vid9 = vid5.derive();
    BOOST_CHECK(vid9.str() == std::string("S$$aaa1234567_0"));

    Vid vid10 = vid7.derive();
    BOOST_CHECK(vid10.str() == std::string("\\S$$**1234567_1 "));

    Vid vid11 = vid10.derive();
    BOOST_CHECK(vid11.str() == std::string("\\S$$**1234567_2 "));
}

enum {
    str1 = 0,
    str2,
    str3,
    str4
};

BOOST_AUTO_TEST_CASE ( test_predef_vid ) {
    std::vector<std::pair<int, std::string> > pd = {
        { str1, "str1" },
        { str2, "averylongstr" },
        { str3, "\\** " },
        { str4, "\\**12345678 " },
    };

    Vid::predef(pd);
    BOOST_CHECK(Vid(str1).str() == "str1");
    BOOST_CHECK(Vid(str2).str() == "averylongstr");
    BOOST_CHECK(Vid(str3).str() == "\\** ");
    BOOST_CHECK(Vid(str4).str() == "\\**12345678 ");
}
