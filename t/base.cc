#include "lib/test.h"

TEST_CASE("base") {

    SECTION("empty ctor") {
        SockAddr sa;
        CHECK(sa.family() == AF_UNSPEC);
    }

    SECTION("copy ctor") {
        SECTION("from unspec") {
            SockAddr tmp;
            SockAddr sa = tmp;
            CHECK(sa.family() == AF_UNSPEC);
        }
        SECTION("from inet4") {
            SockAddr sa = (SockAddr)SockAddr::Inet4("10.10.10.2", 1234);
            CHECK(sa.is_inet4());
            CHECK(sa.inet4().ip() == "10.10.10.2");
        }
        SECTION("from inet6") {
            SockAddr sa = (SockAddr)SockAddr::Inet6("::1", 1234);
            CHECK(sa.is_inet6());
            CHECK(sa.inet6().ip() == "::1");
        }
        #ifndef _WIN32
        SECTION("from unix") {
            SockAddr sa = (SockAddr)SockAddr::Unix("/path");
            CHECK(sa.is_unix());
            CHECK(sa.unix().path() == "/path");
        }
        #endif
    }

    SECTION("==") {
        CHECK(SockAddr::Inet4("127.0.0.1", 10) != SockAddr::Inet6("::1", 10));
    }
}
